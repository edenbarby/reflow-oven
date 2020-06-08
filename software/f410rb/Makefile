PROJ_NAME = project


# CMSIS device specific headers.
DIR_DRIVERS_CMSIS_STM32F4 = lib/cmsis_stm32f4xx
# STM device drivers.
DIR_DRIVERS_STM_STM32F4   = lib/stm_stm32f4xx
# Linker scripts.
DIR_LDSCRIPT              = ldscripts
# Project source files.
DIR_SRC                   = src
# All files produced during the build get dumped here.
DIR_BUILD                 = build


# Generate source list and obj file names for the drivers that need compiling.
SRC := $(DIR_DRIVERS_CMSIS_STM32F4)/src/system_stm32f4xx.c
SRC := $(SRC) $(wildcard $(DIR_DRIVERS_STM_STM32F4)/src/*)
OBJ := $(SRC:.c=.o)
# Add project source list and CMSIS startup code.
SRC := $(wildcard $(DIR_SRC)/*) $(SRC) $(DIR_DRIVERS_CMSIS_STM32F4)/src/startup_stm32f410rx.s


# Tools.
CC   = arm-none-eabi-gcc
GDB  = arm-none-eabi-gdb
PROG = openocd
RM   = rm -f


# Enable a bunch of warnings forcing good coding practice.
CFLAGS  = -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -Wformat=2 -Wstrict-prototypes -Wno-unused-parameter
# Allows linker to garbage collect unused sections of code.
CFLAGS += -ffunction-sections -fdata-sections
# Stops the compiler from breaking code when fucking around the pointers.
CFLAGS += -fno-strict-aliasing
# Optimize for binary size.
CFLAGS += -Os
# Include bulk debugging info as well as GDB debug symbols.
CFLAGS += -g3 -ggdb
# A libc implimentation optimized for embedded systems.
CFLAGS += --specs=nano.specs
# uC specific flags.
CFLAGS += -mcpu=cortex-m4 -mfloat-abi=soft -mthumb
# Defines required for the STM drivers.
CFLAGS += -D STM32F410Rx -D USE_FULL_LL_DRIVER
# Garbage collect unused sections of code.
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,--print-memory-usage


LIBS  =-I $(DIR_DRIVERS_CMSIS_STM32F4)/inc
LIBS +=-I $(DIR_DRIVERS_STM_STM32F4)/inc
LIBS +=-I $(DIR_SRC)

LDSCRIPT = -L $(DIR_LDSCRIPT) -T stm32f410rb_flash.ld


PROG_CFG = -f interface/stlink.cfg -f target/stm32f4x.cfg
PROG_DBG = -c "gdb_port pipe" -c "log_output $(PROJ_NAME)_oocd.log"


.PHONY: all prog clean

all: prog

prog: $(PROJ_NAME).elf
	$(PROG) $(PROG_CFG) -c "program $^ verify reset exit"

debug: $(PROJ_NAME).elf
	$(GDB) -q --eval-command='target remote | $(PROG) $(PROG_CFG) $(PROG_DBG) -c "program $^ verify reset"' $^

$(PROJ_NAME).elf: $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) $^ -o $@ $(LDSCRIPT)

clean:
	$(RM) $(PROJ_NAME).elf
