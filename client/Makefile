#   This makefile is built to allow easy compilation on Linux and Windows
#
#   Copyright (C) 2021 Killian RAIMBAUD (killian.rai@gmail.com)
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; version 2 of the License.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program; if not, write to the Free Software Foundation, Inc.,
#   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

# Recursive search function
recursiveSearch = $(foreach file,$(wildcard $(2)*),$(filter $(subst *,%,$(1)),$(file)) $(call recursiveSearch,$(1),$(file)/))

# Remove duplicate function
removeDuplicates = $(if $(1),$(firstword $(1)) $(call removeDuplicates,$(strip $(filter-out $(firstword $(1)),$(1)))),)

# C Compilers
CC = gcc
WINCC = x86_64-w64-mingw32-gcc

# Compile / Link Flags
CFLAGS += -c -Wall -std=c99 -pedantic-errors -g
LDFLAGS = -lncurses -pthread

WINLDFLAGS = "-L /usr/x86_64-w64-mingw32/lib/ -lmingw32 -lz -llzm -lm -mwindows"

# Main target and filename of the executable
OUT = Client_VMS

# Build Directory
BUILD_DIR = build

# List of all the .c source files to compile
SRC = $(call recursiveSearch,*.c)

# List of all the .o object files to produce
OBJ = $(patsubst %,$(BUILD_DIR)/%,$(SRC:%.c=%.o))
OBJ_DIR = $(call removeDuplicates, $(dir $(OBJ)))

# List of all includes directory
INCLUDES = $(patsubst %, -I %, $(call removeDuplicates, $(dir $(call recursiveSearch,*.h))))
LIBS =

WINLIBS =

# Number of therads available 
CPUS = $(nproc)

multi:
	@$(MAKE) -j$(CPUS) --no-print-directory all

all: $(OBJ_DIR) $(OUT)

$(OBJ_DIR):
	@mkdir -p $@

$(BUILD_DIR)/%.o: %.c
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) $< $(INCLUDES) $(LIBS) -o $@

$(OUT): $(OBJ)
	@echo "Linking $@"
	@$(CC) -o $(OUT) $^ $(LDFLAGS)

clean:
	@echo "Cleaning Build"
	@rm -rf $(BUILD_DIR) $(OUT) $(OUT).exe

rebuild: clean multi
#@$(MAKE) -j$(CPUS) --no-print-directory all

run:
	./$(OUT)

windows:
	@$(MAKE) --no-print-directory rebuild CC=$(WINCC) OUT=$(OUT).exe LIBS=$(WINLIBS) LDFLAGS+=$(WINLDFLAGS)
