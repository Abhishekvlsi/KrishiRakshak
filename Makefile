# KrishiRakshak - Build System for Silicon Labs EFR32MG26
# Silicon Labs Edge Intelligence Challenge 2024

# Project configuration
PROJECT_NAME     = krishirakshak
TARGET           = EFR32MG24B310F1536IM48
COMPILER         = arm-none-eabi-gcc
SDK_PATH         = $(GSDK_PATH)
OUTPUT_DIR       = build
BIN_DIR          = bin

# Source files
SRC_DIR          = src/firmware
SRCS            := $(wildcard $(SRC_DIR)/*.c)
OBJS            := $(SRCS:$(SRC_DIR)/%.c=$(OUTPUT_DIR)/%.o)

# Include paths
INCLUDES        := -I$(SRC_DIR) \
                   -I$(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG24/Include \
                   -I$(SDK_PATH)/platform/CMSIS/Core/Include \
                   -I$(SDK_PATH)/platform/emlib/inc \
                   -I$(SDK_PATH)/hardware/kit/common/bsp \
                   -I$(SDK_PATH)/hardware/kit/common/drivers

# Compiler flags
CFLAGS          := -mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 \
                   -D$(TARGET) -DUSE_CMSIS_NN -DUSE_MVP_ACCELERATION \
                   -O2 -g -Wall -Wextra -Wno-unused-parameter \
                   -ffunction-sections -fdata-sections \
                   -std=gnu99 $(INCLUDES)

# Linker flags
LDFLAGS         := -mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 \
                   -T"$(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG24/Source/GCC/efr32mg24.ld" \
                   -Xlinker --gc-sections -Xlinker -Map="$(OUTPUT_DIR)/$(PROJECT_NAME).map" \
                   --specs=nano.specs -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group

# Targets
.PHONY: all clean flash debug sim

all: $(BIN_DIR)/$(PROJECT_NAME).bin

$(OUTPUT_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OUTPUT_DIR)
	$(COMPILER) $(CFLAGS) -c $< -o $@

$(OUTPUT_DIR)/$(PROJECT_NAME).elf: $(OBJS)
	@mkdir -p $(OUTPUT_DIR)
	$(COMPILER) $(LDFLAGS) -o $@ $^

$(BIN_DIR)/$(PROJECT_NAME).bin: $(OUTPUT_DIR)/$(PROJECT_NAME).elf
	@mkdir -p $(BIN_DIR)
	arm-none-eabi-objcopy -O binary $< $@

$(BIN_DIR)/$(PROJECT_NAME).hex: $(OUTPUT_DIR)/$(PROJECT_NAME).elf
	@mkdir -p $(BIN_DIR)
	arm-none-eabi-objcopy -O ihex $< $@

# Build targets
build: $(BIN_DIR)/$(PROJECT_NAME).bin $(BIN_DIR)/$(PROJECT_NAME).hex

clean:
	rm -rf $(OUTPUT_DIR) $(BIN_DIR)

flash: $(BIN_DIR)/$(PROJECT_NAME).hex
	commander flash $(BIN_DIR)/$(PROJECT_NAME).hex --device $(TARGET)

debug: $(OUTPUT_DIR)/$(PROJECT_NAME).elf
	arm-none-eabi-gdb $(OUTPUT_DIR)/$(PROJECT_NAME).elf

# Simulation targets
sim:
	cd src/simulation && python edge_simulator.py

sim-data:
	cd src/simulation && python data_generator.py

sim-benchmark:
	cd src/simulation && python edge_simulator.py --benchmark

# Analysis targets
size: $(OUTPUT_DIR)/$(PROJECT_NAME).elf
	arm-none-eabi-size $(OUTPUT_DIR)/$(PROJECT_NAME).elf

analyze: $(OUTPUT_DIR)/$(PROJECT_NAME).elf
	arm-none-eabi-objdump -h $(OUTPUT_DIR)/$(PROJECT_NAME).elf
	arm-none-eabi-nm -S --size-sort $(OUTPUT_DIR)/$(PROJECT_NAME).elf | tail -20

# Dependency generation
deps: $(SRCS)
	$(COMPILER) $(CFLAGS) -MM $(SRCS) > $(OUTPUT_DIR)/dependencies.mk

-include $(OUTPUT_DIR)/dependencies.mk

# Help target
help:
	@echo "KrishiRakshak Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all        - Build binary and hex files"
	@echo "  build      - Build project"
	@echo "  clean      - Clean build artifacts"
	@echo "  flash      - Flash to device"
	@echo "  debug      - Start debug session"
	@echo "  sim        - Run Python simulation"
	@echo "  sim-data   - Generate synthetic data"
	@echo "  size       - Show memory usage"
	@echo "  analyze    - Analyze binary"
	@echo "  deps       - Generate dependencies"
	@echo "  help       - Show this help"

# Print build information
info:
	@echo "Project: $(PROJECT_NAME)"
	@echo "Target: $(TARGET)"
	@echo "Compiler: $(COMPILER)"
	@echo "Sources: $(words $(SRCS)) files"
	@echo "SDK Path: $(SDK_PATH)"
