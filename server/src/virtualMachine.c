/*
    Copyright (C) 2021 Killian RAIMBAUD [Asayu] (killian.rai@gmail.com)
    Copyright (C) 2021 Francois Meunier

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#define _GNU_SOURCE

#include "virtualMachine.h"
#include "linkedList.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <linux/limits.h>
#include <sys/select.h>
#include <sys/termios.h>
#include <string.h>

#define VM_IMAGE_OFFSET 0x3000

/* Registers */
enum {
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC, /* program counter */
    R_COND,
    R_COUNT
};

/* Opcodes */
enum {
    OP_BR = 0, /* branch */
    OP_ADD,    /* add  */
    OP_LD,     /* load */
    OP_ST,     /* store */
    OP_JSR,    /* jump register */
    OP_AND,    /* bitwise and */
    OP_LDR,    /* load register */
    OP_STR,    /* store register */
    OP_RTI,    /* unused */
    OP_NOT,    /* bitwise not */
    OP_LDI,    /* load indirect */
    OP_STI,    /* store indirect */
    OP_JMP,    /* jump */
    OP_RES,    /* reserved (unused) */
    OP_LEA,    /* load effective address */
    OP_TRAP    /* execute trap */
};

/* Condition Flags */
enum {
    FL_POS = 1 << 0, /* P */
    FL_ZRO = 1 << 1, /* Z */
    FL_NEG = 1 << 2 /* N */
};

/* Memory Mapped Registers */
enum {
    MR_KBSR = 0xFE00, /* keyboard status */
    MR_KBDR = 0xFE02  /* keyboard data */
};

/* TRAP Codes */
enum {
    TRAP_GETC  = 0x20,  /* get character from keyboard, not echoed onto the terminal */
    TRAP_OUT   = 0x21,  /* output a character */
    TRAP_PUTS  = 0x22,  /* output a word string */
    TRAP_IN    = 0x23,  /* get character from keyboard, echoed onto the terminal */
    TRAP_PUTSP = 0x24,  /* output a byte string */
    TRAP_HALT  = 0x25   /* halt the program */
};

/* Sign Extend */
uint16_t sign_extend(uint16_t x, int bit_count) {
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

/* Swap */
uint16_t swap16(uint16_t x) {
    return (x << 8) | (x >> 8);
}

/* Update Flags */
void update_flags(uint16_t reg[R_COUNT], uint16_t r) {
    if (reg[r] == 0) {
        reg[R_COND] = FL_ZRO;
    } else if (reg[r] >> 15) { /* a 1 in the left-most bit indicates negative */
        reg[R_COND] = FL_NEG;
    } else {
        reg[R_COND] = FL_POS;
    }
}

/* Read Image File */
bool read_image_file(uint16_t *memory, char *image_path, uint16_t *origin) {
	char fich[PATH_MAX];
    FILE* file;
    uint16_t max_read, *p;
	
    strcpy(fich, image_path);
  	file = fopen(fich, "rb");
 
    if (!file) return false;
    /* the origin tells us where in memory to place the image */
   	*origin = VM_IMAGE_OFFSET;

    /* we know the maximum file size so we only need one fread */
    max_read = UINT16_MAX - *origin;
    p = memory + *origin;
    /* size_t read = fread(p, sizeof(uint16_t), max_read, file); */
    fread(p, sizeof(uint16_t), max_read, file);
    
    /* swap to little endian ????
    while (read-- > 0) {
        printf("\n p * BIG = %x",*p);
        *p = swap16(*p);
        printf("\n p * LITTLE = %x",*p);
        ++p;
    }
    */
    return true;
}


/* Check Key */
uint16_t check_key() {
    fd_set readfds;
    struct timeval timeout;

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
}

/* Memory Access */
void mem_write(uint16_t *memory, uint16_t address, uint16_t val) {
    memory[address] = val;
}

uint16_t mem_read( uint16_t *memory, uint16_t address) {
    if (address == MR_KBSR) { /* mmio */
        if (check_key()) {
            memory[MR_KBSR] = (1 << 15);
            memory[MR_KBDR] = getchar();
        } else {
            memory[MR_KBSR] = 0;
        }
    }
    return memory[address];
}

/* Input Buffering */
struct termios original_tio;

void disable_input_buffering() {
    struct termios new_tio;
    
    tcgetattr(STDIN_FILENO, &original_tio);
    new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void restore_input_buffering() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

/* Handle Interrupt */
void handle_interrupt(int signal) {
    restore_input_buffering();
    //pthread_mutex_lock(&consoleState);
    printf("\n");
    //pthread_mutex_unlock(&consoleState);
    exit(-2);
}

/*
  #######################################
  #
  # Execute le fichier de code .obj 
  #
*/
int executeFile(VirtualMachine* VM, char* sourcefname){
/* Memory Storage */
/* 65536 locations */
	uint16_t *memory;
	uint16_t origin, PC_START;
	
/* Register Storage */
	uint16_t reg[R_COUNT];

    int running = 1;
    char c, char1, char2;

	memory = VM->ptrDebutVM;
    if (!read_image_file(memory, sourcefname, &origin)) {
        pthread_mutex_lock(VM->consoleState);
        dprintf(*VM->console, "VM %d : Failed to load image: %s\n", VM->noVM, sourcefname);
        pthread_mutex_unlock(VM->consoleState);
        VM->busy = false;
        return 0;
    }
    
    /* Setup */
    VM->busy = true;
    signal(SIGINT, handle_interrupt);
    
    /* TEMP
    disable_input_buffering(); */

    /* set the PC to starting position */
    /* at  ptr->VM.ptrDebutVM + 0x3000 is the default  */
    PC_START = origin;
    reg[R_PC] = PC_START;

    while (running) {
        /* FETCH */
        uint16_t instr = mem_read(memory, reg[R_PC]++);
        uint16_t op = instr >> 12;

        switch (op) {
            case OP_ADD:
                /* ADD */
                {
                    /* destination register (DR) */
                    uint16_t r0 = (instr >> 9) & 0x7;
                    /* first operand (SR1) */
                    uint16_t r1 = (instr >> 6) & 0x7;
                    /* whether we are in immediate mode */
                    uint16_t imm_flag = (instr >> 5) & 0x1;
                
                    if (imm_flag) {
                        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                        reg[r0] = reg[r1] + imm5;
                    } else {
                        uint16_t r2 = instr & 0x7;
                        reg[r0] = reg[r1] + reg[r2];
                        pthread_mutex_lock(VM->consoleState);
                        dprintf(*VM->console, "\nVM %d : add reg[r0] (sum) = %d", VM->noVM, reg[r0]);
                        pthread_mutex_unlock(VM->consoleState);
                        /* printf("\t add reg[r1] (sum avant) = %d", reg[r1]); */
                        /* printf("\t add reg[r2] (valeur ajoutee) = %d", reg[r2]); */
                    }
                
                    update_flags(reg, r0);
                }

                break;
            case OP_AND:
                /* AND */
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                    uint16_t imm_flag = (instr >> 5) & 0x1;
                
                    if (imm_flag) {
                        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                        reg[r0] = reg[r1] & imm5;
                    } else {
                        uint16_t r2 = instr & 0x7;
                        reg[r0] = reg[r1] & reg[r2];
                    }
                    update_flags(reg, r0);
                }

                break;
            case OP_NOT:
                /* NOT */
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                
                    reg[r0] = ~reg[r1];
                    update_flags(reg, r0);
                }

                break;
            case OP_BR:
                /* BR */
                {
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    uint16_t cond_flag = (instr >> 9) & 0x7;
                    if (cond_flag & reg[R_COND]) {
                        reg[R_PC] += pc_offset;
                    }
                }

                break;
            case OP_JMP:
                /* JMP */
                {
                    /* Also handles RET */
                    uint16_t r1 = (instr >> 6) & 0x7;
                    reg[R_PC] = reg[r1];
                }

                break;
            case OP_JSR:
                /* JSR */
                {
                    uint16_t long_flag = (instr >> 11) & 1;
                    reg[R_R7] = reg[R_PC];
                    if (long_flag) {
                        uint16_t long_pc_offset = sign_extend(instr & 0x7FF, 11);
                        reg[R_PC] += long_pc_offset;  /* JSR */
                    } else {
                        uint16_t r1 = (instr >> 6) & 0x7;
                        reg[R_PC] = reg[r1]; /* JSRR */
                    }
                    break;
                }

                break;
            case OP_LD:
                /* LD */
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    reg[r0] = mem_read(memory, reg[R_PC] + pc_offset);
                    update_flags(reg, r0);
                }

                break;
            case OP_LDI:
                /* LDI */
                {
                    /* destination register (DR) */
                    uint16_t r0 = (instr >> 9) & 0x7;
                    /* PCoffset 9*/
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    /* add pc_offset to the current PC, look at that memory location to get the final address */
                    reg[r0] = mem_read(memory, mem_read(memory, reg[R_PC] + pc_offset));
                    update_flags(reg, r0);
                }

                break;
            case OP_LDR:
                /* LDR */
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                    uint16_t offset = sign_extend(instr & 0x3F, 6);
                    reg[r0] = mem_read(memory, reg[r1] + offset);
                    update_flags(reg, r0);
                }

                break;
            case OP_LEA:
                /* LEA */
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    reg[r0] = reg[R_PC] + pc_offset;
                    update_flags(reg, r0);
                }

                break;
            case OP_ST:
                /* ST */
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    mem_write(memory, reg[R_PC] + pc_offset, reg[r0]);
                }

                break;
            case OP_STI:
                /* STI */
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    mem_write(memory, mem_read(memory, reg[R_PC] + pc_offset), reg[r0]);
                }

                break;
            case OP_STR:
                /* STR */
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                    uint16_t offset = sign_extend(instr & 0x3F, 6);
                    mem_write(memory, reg[r1] + offset, reg[r0]);
                }

                break;
            case OP_TRAP:
                /* TRAP */
                switch (instr & 0xFF) {
                    case TRAP_GETC:
                        /* TRAP GETC */
                        /* read a single ASCII char */
                        reg[R_R0] = (uint16_t)getchar();

                        break;
                    case TRAP_OUT:
                        /* TRAP OUT */
                        putc((char)reg[R_R0], stdout);
                        fflush(stdout);

                        break;
                    case TRAP_PUTS:
                        /* TRAP PUTS */
                        {
                            /* one char per word */
                            uint16_t* c = memory + reg[R_R0];
                            while (*c) {
                                putc((char)*c, stdout);
                                ++c;
                            }
                            fflush(stdout);
                        }

                        break;
                    case TRAP_IN:
                        /* TRAP IN */
                        {
                            pthread_mutex_lock(VM->consoleState);
                            dprintf(*VM->console, "VM %d : Enter a character: ", VM->noVM);
                            pthread_mutex_unlock(VM->consoleState);
                            c = getchar();
                            putc(c, stdout);
                            reg[R_R0] = (uint16_t)c;
                        }

                        break;
                    case TRAP_PUTSP:
                        /* TRAP PUTSP */
                        {
                            /* one char per byte (two bytes per word)
                               here we need to swap back to
                               big endian format */
                            uint16_t* c = memory + reg[R_R0];
                            while (*c) {
                                char1 = (*c) & 0xFF;
                                putc(char1, stdout);
                                char2 = (*c) >> 8;
                                if (char2) putc(char2, stdout);
                                ++c;
                            }
                            fflush(stdout);
                        }

                        break;
                    case TRAP_HALT:
                        /* TRAP HALT */
                        puts("\n HALT");
                        fflush(stdout);
                        running = 0;

                        break;
                }

                break;
            case OP_RES:
            case OP_RTI:
            default:
                /* BAD OPCODE */
                abort();

                break;
        }
    }

    /* Shutdown */
    VM->busy = false;
    /* restore_input_buffering(); */
    
    return(1);
}

void* virtualMachine(void* args) {
    VirtualMachine* self = args;

    self->busy = false;
    while (!self->kill || self->binaryList) {
        if (self->binaryList) {
            
            pthread_mutex_lock(self->consoleState);
            dprintf(*self->console, "VM %d executing ! %s\n", self->noVM, (char*)self->binaryList->data);
            pthread_mutex_unlock(self->consoleState);
            if (self->binaryList->data == NULL)
                printf("WUT\n");
            executeFile(self, self->binaryList->data); /* Executing Current Job */
            if (self->binaryList->data == NULL)
                printf("DOUBLE WUT\n");
            
            DeleteLinkedListNode(&self->binaryList);   /* Free completed Job */
        }
    }

    return NULL;
}