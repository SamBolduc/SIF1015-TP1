# Recursive Wildcard function
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2)$(filter $(subst *,%,$2),$d))

# Remove duplicate function
uniq = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))

# C Compiler
CC = gcc

# Target
OUT = VMS

# Flags
CFLAGS += -c -Wall -g
LDFLAGS += -lpthread

SRC = $(call rwildcard,,*.c)

OBJ = $(patsubst %,build/%,$(SRC:%.c=%.o))
OBJ_DIR = $(call uniq, $(dir $(OBJ)))

INCLUDES = $(patsubst %, -I %, $(call uniq, $(dir $(call rwildcard,,*.h))))

# Number of therads available
CPUS = $(nproc)

multi:
	@$(MAKE) -j$(CPUS) --no-print-directory all

rebuild: clean multi

all: $(OBJ_DIR) $(OUT)

$(OBJ_DIR):
	@mkdir -p $@

build/%.o: %.c
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) $< $(INCLUDES) -o $@

$(OUT): $(OBJ)
	@echo "Linking $@"
	@$(CC) -o $(OUT) $^ $(LDFLAGS)

clean:
	@echo "Cleaning Build"
	@rm -rf build $(OUT)
