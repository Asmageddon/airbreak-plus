# stm32-unlocked.bin: patch-airsense
# 	./patch-airsense stm32.bin $@

SRC=patches
BUILD=build

all: $(BUILD)/stm32-patched.bin $(BUILD)/stm32-asv.bin

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/stm32-patched.bin: patch-airsense $(BUILD)/common_code.bin $(BUILD)/graph.bin
	export PATCH_CODE=1 && ./patch-airsense stm32.bin $@

$(BUILD)/stm32-asv.bin: patch-airsense $(BUILD)/common_code.bin $(BUILD)/graph.bin $(BUILD)/squarewave.bin $(BUILD)/asv_task_wrapper.bin $(BUILD)/wrapper_limit_max_pdiff.bin
	export PATCH_CODE=1 && export PATCH_S=1 && export PATCH_ASV_TASK_WRAPPER=1 && export PATCH_VAUTO_WRAPPER=1 && ./patch-airsense stm32.bin $@

binaries: $(BUILD)/common_code.bin $(BUILD)/graph.bin $(BUILD)/squarewave.bin $(BUILD)/asv_task_wrapper.bin $(BUILD)/wrapper_limit_max_pdiff.bin

serve:
	mkdocs serve
deploy:
	mkdocs gh-deploy

# There are decent distances between the different patches, 
# but if you substantially increase the amount of code, beware collisions.
# I've already had several happen in the past, whoops :F

$(BUILD)/common_code.elf: $(BUILD)/common_code.o $(BUILD)/stubs.o
common_code-offset := 0x80fe000

# The graphing is too large to fit directly in the location at 0x8067d2c,
# so it is in high in the flash and the function pointer is fixed up at 0x80f9c88
$(BUILD)/graph.elf: $(BUILD)/graph.o $(BUILD)/stubs.o
graph-offset := 0x80fcd40
graph-extra := --just-symbols=$(BUILD)/common_code.elf

$(BUILD)/squarewave.elf: $(BUILD)/squarewave.o $(BUILD)/stubs.o
squarewave-offset := 0x80fd200
squarewave-extra := --just-symbols=$(BUILD)/common_code.elf

$(BUILD)/asv_task_wrapper.elf: $(BUILD)/asv_task_wrapper.o $(BUILD)/stubs.o
asv_task_wrapper-offset := 0x80fdf00
asv_task_wrapper-extra := --just-symbols=$(BUILD)/common_code.elf

$(BUILD)/wrapper_limit_max_pdiff.elf: $(BUILD)/wrapper_limit_max_pdiff.o $(BUILD)/stubs.o
wrapper_limit_max_pdiff-offset := 0x80ff000
wrapper_limit_max_pdiff-extra := --just-symbols=$(BUILD)/common_code.elf

# If there is a new version of the ghidra XML, the stubs.S
# file will be regenerated so that the addresses and functions
# are at the correct address in the ELF image.
#stubs.S: stm32.bin.xml
#	./ghidra2stubs < $< > $@


CROSS ?= arm-none-eabi-
CC := $(CROSS)gcc
AS := $(CC)
LD := $(CROSS)ld
OBJCOPY := $(CROSS)objcopy

CFLAGS ?= \
	-g \
	-Os \
	-mcpu=cortex-m4 \
	-mhard-float \
	-mfp16-format=ieee \
	-mthumb \
	-W \
	-Wall \
	-Wno-unused-result \
	-Wno-unused-parameter \
	-Wno-unused-variable \
	-nostdlib \
	-nostdinc \
	-ffreestanding \

ASFLAGS ?= $(CFLAGS)

LDFLAGS ?= \
	--nostdlib \
	--no-dynamic-linker \
	--Ttext $($*-offset) \
	$($*-extra) \
	--entry start \
	--sort-section=name \

# TODO: Sort sections by name, lay out main before the rest, to avoid inlining everything

# $(BUILD)/shared_code.o: $(BUILD)/shared_code.c
# 	$(CC) $(CFLAGS) -static -shared -c -o $@ $<
$(BUILD)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(BUILD)/%.o: $(SRC)/%.S
	$(AS) $(ASFLAGS) -c -o $@ $<
$(BUILD)/%.elf: | $(BUILD)
	$(LD) $(LDFLAGS) -o $@ $^

$(BUILD)/%.bin: $(BUILD)/%.elf
	$(OBJCOPY) -Obinary $< $@


# eeprom_stub - standalone CDX replacement for s10 platform
#
# Build: make eeprom_stub
# Output: build/eeprom_stub_nocrc.bin (raw, for patched bootloader)
#         build/eeprom_stub_full.bin  (768KB + CRC, for stock bootloader)

EEPROM_STUB_OFFSET ?= 0x08040000

EEPROM_STUB_OBJS := $(patsubst $(SRC)/%.c,$(BUILD)/%.o,$(wildcard $(SRC)/eeprom_stub*.c))

# The stub uses its own linker script
$(BUILD)/eeprom_stub.elf: $(EEPROM_STUB_OBJS) | $(BUILD)
	$(LD) --nostdlib \
		-T $(SRC)/eeprom_stub.ld \
		--defsym=STUB_FLASH_ORIGIN=$(EEPROM_STUB_OFFSET) \
		-o $@ $(EEPROM_STUB_OBJS)

$(BUILD)/eeprom_stub_nocrc.bin: $(BUILD)/eeprom_stub.elf
	$(OBJCOPY) -Obinary $< $@

eeprom_stub: $(BUILD)/eeprom_stub_nocrc.bin $(BUILD)/eeprom_stub_full.bin
	@echo "EEPROM stub built:"
	@echo "  $(BUILD)/eeprom_stub_nocrc.bin  (raw, for patched bootloader)"
	@echo "  $(BUILD)/eeprom_stub_full.bin   (768KB + CRC, for stock bootloader)"
	@$(CROSS)size $(BUILD)/eeprom_stub.elf

# Full CDX image: pad to region size, fix CRC16 for bootloader validation
$(BUILD)/eeprom_stub_full.bin: $(BUILD)/eeprom_stub_nocrc.bin
	@python3 python/fix_crc.py $< -o $@ --pad 0xC0000


clean:
	$(RM) $(BUILD)/*
