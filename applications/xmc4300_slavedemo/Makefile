
TARGET = xmc4300-ecat-slave

DEVICE = XMC4300
VARIANT = F100x256

EC_SII = sii_eeprom.bin

SRC = \
	main.c \
	objectlist.c \
	soes.c \
	../../soes/esc.c \
	../../soes/esc_coe.c \
	../../soes/esc_eep.c \
	../../soes/hal/xmc4/esc_hw.c \
	../../soes/hal/xmc4/esc_hw_eep.c \
	Libraries/XMCLib/src/xmc_gpio.c \
	Libraries/XMCLib/src/xmc4_gpio.c \
	Libraries/XMCLib/src/xmc4_scu.c \
	Libraries/XMCLib/src/xmc_ecat.c \
	Libraries/XMCLib/src/xmc4_flash.c \
	Libraries/XMCLib/src/xmc_fce.c \
	Libraries/Newlib/syscalls.c \
	Startup/system_$(DEVICE).c \

ASRC = \
	Startup/startup_$(DEVICE).S \

BINS = \
	$(EC_SII)\


LINKER_SCRIPT = ./linker_script.ld

# JLink options
JLINK_SPEED = 1000
JLINK_IFACE = swd

# Define all object files.
OBJ = $(SRC:.c=.o) $(ASRC:.S=.o) $(BINS:.bin=.o) 

# Define all listing files.
LST = $(ASRC:.S=.lst) $(SRC:.c=.lst)

# Define all dependency files.
DEP = $(ASRC:.S=.d) $(SRC:.c=.d)

CPPFLAGS = -D$(DEVICE)_$(VARIANT)
CPPFLAGS += -MMD -MP -MF $(@:.o=.d) -MT $(@)

CPPFLAGS += -I./Libraries/XMCLib/inc
CPPFLAGS += -I./Libraries/CMSIS/Include
CPPFLAGS += -I./Libraries/CMSIS/Infineon/$(DEVICE)_series/Include
CPPFLAGS += -I.
CPPFLAGS += -I./Libraries
CPPFLAGS += -I../../soes
CPPFLAGS += -I../../soes/hal/xmc4
CPPFLAGS += -I../../soes/include/sys/gcc

ARCHFLAGS = -mfloat-abi=softfp -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mthumb

OBJFLAGS = -Wall -fmessage-length=0 -Wa,-adhlns=$(@:.o=.lst) $(ARCHFLAGS)

CFLAGS = $(CPPFLAGS) $(OBJFLAGS) -Os -ffunction-sections -fdata-sections  -std=gnu99 -pipe
ASFLAGS = -x assembler-with-cpp $(CPPFLAGS) $(OBJFLAGS)

LDFLAGS = -T$(LINKER_SCRIPT) -nostartfiles -Xlinker --gc-sections
LDFLAGS += -specs=nano.specs -specs=nosys.specs
LDFLAGS += -Wl,-Map,$(TARGET).map
LDFLAGS += $(ARCHFLAGS)

# Define programs and commands.
SHELL = sh
REMOVE = rm -f
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE = arm-none-eabi-size
JLINK = JLinkExe

# Define Messages
# English
MSG_SIZE = Size: 
MSG_FLASH_FILE = Creating load file for Flash:
MSG_FLASH = Programming target flash memory:
MSG_FLASH_ERASE = Erasing target flash memory:
MSG_EXTENDED_LISTING = Creating Extended Listing:
MSG_LINKING = Linking:
MSG_COMPILING = Compiling:
MSG_ASSEMBLING = Assembling:
MSG_OBJCOPY = ObjCopy:
MSG_CLEANING = Cleaning project:

# Default target.
all: $(TARGET).hex size

# Compile: create object files from C source files.
%.o : %.c
	@echo
	@echo $(MSG_COMPILING) $<
	$(CC) -c $(CFLAGS) -o $@ $< 

# Assemble: create object files from assembler source files.
%.o : %.S
	@echo
	@echo $(MSG_ASSEMBLING) $<
	$(CC) -c $(ASFLAGS) -o $@ $<

# ObjCopy: create object files from binary files.
%.o : %.bin
	@echo
	@echo $(MSG_OBJCOPY) $<
	$(OBJCOPY) -I binary -O elf32-littlearm -B arm $< $@

# Link: create ELF output file from object files.
$(TARGET).elf: $(OBJ)
	@echo
	@echo $(MSG_LINKING) $@
	$(CC) $(LDFLAGS) -o $@ $(OBJ) 

# Create final output file from ELF output file.
$(TARGET).hex: $(TARGET).elf
	@echo
	@echo $(MSG_FLASH_FILE) $@
	$(OBJCOPY) -O ihex $< $@

# Create extended listing file from ELF output file.
$(TARGET).lss: $(TARGET).elf
	@echo
	@echo $(MSG_EXTENDED_LISTING) $@
	$(OBJDUMP) -h -S $(TARGET).elf > $(TARGET).lss

# Display size of file.
size: $(TARGET).elf
	@echo
	@echo $(MSG_SIZE)
	$(SIZE) --format=berkeley $(TARGET).elf
	@echo

# Flash the target device.
flash: $(TARGET).hex
	@echo
	@echo $(MSG_FLASH)
	echo -e "device $(DEVICE)-$(VARIANT)\nspeed $(JLINK_SPEED)\nsi $(JLINK_IFACE)\nloadfile $(TARGET).hex\ng\nqc\n" | $(JLINK)

# Erase the target flash
erase:
	@echo
	@echo $(MSG_FLASH_ERASE)
	echo -e "device $(DEVICE)-$(VARIANT)\nspeed $(JLINK_SPEED)\nsi $(JLINK_IFACE)\nerase\nqc\n" | $(JLINK)

# Target: clean project.
clean:
	@echo
	@echo $(MSG_CLEANING)
	$(REMOVE) $(TARGET).map
	$(REMOVE) $(TARGET).elf
	$(REMOVE) $(TARGET).hex
	$(REMOVE) $(TARGET).lss
	$(REMOVE) $(OBJ)
	$(REMOVE) $(LST)
	$(REMOVE) $(DEP)

# Include the dependency files.
-include $(DEP)

# Listing of phony targets.
.PHONY: all clean size flash erase

