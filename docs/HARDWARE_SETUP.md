
# KrishiRakshak - Hardware Setup Guide

##  Required Components

### Silicon Labs Development Kits
| Component | Part Number | Purpose | Where to Obtain |
|-----------|-------------|---------|-----------------|
| **EFR32MG24 Dev Kit** | SLWRB4181A | Edge AI Processing | Silicon Labs Store |
| **SiWx917 Dev Kit** | SLWRB4310A | Wireless Communication | Silicon Labs Store |
| **Debug Adapter** | SLWSTK6021B | Programming & Debugging | Silicon Labs Store |

### Sensors & Peripherals
| Sensor | Model | Interface | Notes |
|--------|-------|-----------|-------|
| **Soil Moisture** | Capacitive Sensor | Analog (ADC) | 3.3V compatible |
| **Temperature/Humidity** | SHT31 | I2C | High accuracy |
| **MEMS Microphone** | SPH0645LM4H | I2S/PDM | Bottom port |
| **Solar Panel** | 5W, 6V | Power Input | With charge controller |
| **LiPo Battery** | 3.7V, 2000mAh | Power Storage | JST connector |

## Hardware Connections

### EFR32MG24 Pin Mapping
| Peripheral | Pin | Function | Connection |
|------------|-----|----------|------------|
| **Soil Moisture** | PA0 | ADC0 | Sensor output |
| **SHT31 SDA** | PC4 | I2C0_SDA | Sensor data |
| **SHT31 SCL** | PC5 | I2C0_SCL | Sensor clock |
| **Microphone CLK** | PD11 | I2S_CLK | Audio clock |
| **Microphone DATA** | PD12 | I2S_DATA | Audio data |
| **Microphone WS** | PD13 | I2S_WS | Word select |
| **SiWx917 UART TX** | PA5 | UART1_TX | To SiWx917 RX |
| **SiWx917 UART RX** | PA6 | UART1_RX | From SiWx917 TX |
| **SiWx917 Control** | PB0 | GPIO | Wake/Control |

### Power Connections
```
SOLAR PANEL (6V) → CHARGE CONTROLLER → LiPo BATTERY (3.7V)
                                      ↓
                               EFR32MG24 (3.3V)
                               SiWx917 (3.3V)
                               Sensors (3.3V)
```

##  Power Management Setup

### Solar Charger Configuration
```c
// Battery management parameters
#define BATTERY_FULL_VOLTAGE    4.2f    // 3.7V LiPo full
#define BATTERY_LOW_VOLTAGE     3.3f    // Minimum operating voltage
#define SOLAR_CHARGE_CURRENT    500     // 500mA max charge current
```

### Power Mode Configuration
```c
typedef enum {
    POWER_MODE_ACTIVE = 0,      // All systems active
    POWER_MODE_SENSING,         // Sensors + EFR32 active
    POWER_MODE_SLEEP,           // Only EFR32 in low-power
    POWER_MODE_DEEP_SLEEP       // Minimum power
} power_mode_t;
```

## Development Environment Setup

### 1. Software Installation
```bash
# Install Simplicity Studio 5
# Download from Silicon Labs website

# Install GSDK 5.8.2
# Through Simplicity Studio Package Manager

# Install necessary plugins:
# - TensorFlow Lite Micro
# - CMSIS-NN
# - Energy Profiler
# - Network Analyzer
```

### 2. Project Configuration
```c
// Project settings for EFR32MG24
#define SL_TFLITE_MICRO_ENABLE     1
#define SL_MVP_ENABLE             1
#define SL_CMSIS_NN_ENABLE        1
#define SL_WIRELESS_ENABLE        1

// Memory configuration
#define TFLITE_TENSOR_ARENA_SIZE   (20 * 1024)  // 20KB for ML
#define SENSOR_BUFFER_SIZE         (2 * 1024)   // 2KB for sensor data
```

### 3. MVP Configuration
```c
// Enable MVP in project settings
#pragma GCC target ("arch=armv8.1-m.main+mve.fp")
  
// MVP-optimized memory layout
__attribute__((section(".ram_mvp"))) int8_t ml_weights[2979];
__attribute__((section(".ram_mvp"))) int8_t ml_biases[64+32+16+3];
```

##  Testing & Validation

### 1. Hardware Test Sequence
```c
void hardware_self_test(void) {
    // Test each sensor
    test_soil_sensor();
    test_temperature_sensor();
    test_humidity_sensor();
    test_microphone();
    
    // Test ML inference
    test_ml_model();
    
    // Test wireless communication
    test_wireless_connection();
    
    // Test power management
    test_power_modes();
}
```

### 2. Sensor Calibration
```c
// Soil moisture calibration
void calibrate_soil_sensor(void) {
    // Dry calibration (in air)
    uint16_t dry_value = read_adc(SOIL_SENSOR_PIN);
    
    // Wet calibration (in water)
    uint16_t wet_value = read_adc(SOIL_SENSOR_PIN);
    
    // Store calibration values
    store_calibration(dry_value, wet_value);
}
```

### 3. Power Consumption Measurement
```bash
# Using Silicon Labs Energy Profiler
# Expected power measurements:
# - Sleep mode: <20 μA
# - Sensor reading: ~5 mA
# - ML inference: ~15 mA
# - Wireless transmission: ~50 mA
```

##  Firmware Flashing

### 1. Build Process
```bash
# In Simplicity Studio
# 1. Import project from GitHub
# 2. Set build configuration: Release + MVP optimized
# 3. Build project
# 4. Generate .hex and .bin files
```

### 2. Flashing Steps
```bash
# Using J-Link debugger
$ JLinkExe -device EFR32MG24B310 -if SWD -speed 4000
J-Link> erase
J-Link> loadfile krishirakshak_firmware.hex
J-Link> reset
J-Link> go
```

### 3. Verification
```c
// Firmware version check
void check_firmware_version(void) {
    printf("KrishiRakshak Firmware v1.0\n");
    printf("Built: %s %s\n", __DATE__, __TIME__);
    printf("MVP Enabled: %s\n", SL_MVP_ENABLE ? "YES" : "NO");
    printf("ML Model: %s\n", ML_MODEL_VERSION);
}
```

##  Debugging & Troubleshooting

### Common Issues
1. **MVP Not Accelerating**
   - Check compiler flags for MVE support
   - Verify memory sections for MVP data
   - Confirm CMSIS-NN with MVP enabled

2. **High Power Consumption**
   - Verify all peripherals in sleep mode
   - Check for floating pins
   - Validate power mode transitions

3. **Wireless Connectivity Issues**
   - Verify UART connections between chips
   - Check antenna matching
   - Validate power supply stability

### Debug Tools
```c
// Enable debug output
#define DEBUG_ENABLE 1

#if DEBUG_ENABLE
#define DEBUG_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...)
#endif

// Usage
DEBUG_PRINT("Inference time: %d ms\n", inference_time);
```

## Performance Optimization

### 1. Memory Optimization
```c
// Optimized tensor arena for TFLite Micro
__attribute__((aligned(16))) 
static int8_t tensor_arena[TFLITE_TENSOR_ARENA_SIZE];

// MVP-optimized weight layout
__attribute__((section(".ram_mvp"))) 
static const int8_t model_weights[] = { ... };
```

### 2. Power Optimization
```c
void optimize_power_consumption(void) {
    // Reduce sensor sampling rate when not needed
    set_sensor_sampling_rate(LOW_POWER_RATE);
    
    // Use lowest possible clock speed
    set_cpu_clock_speed(LOWEST_STABLE_CLOCK);
    
    // Aggressive sleep between operations
    enter_deep_sleep_between_cycles();
}
```

### 3. Inference Optimization
```c
void optimize_inference(void) {
    // Use MVP-accelerated CMSIS-NN functions
    enable_mvp_acceleration();
    
    // Batch process sensor data when possible
    enable_sensor_batching();
    
    // Use quantized model with 8-bit arithmetic
    use_quantized_inference();
}
```

##  Safety & Compliance

### Environmental Protection
- **Enclosure:** IP67 rated for outdoor use
- **Temperature Range:** -20°C to 60°C operation
- **UV Resistance:** UV-stabilized plastic housing
- **Lightning Protection:** TVS diodes on all external connections

### Electrical Safety
- **Reverse Polarity Protection:** On power inputs
- **Overvoltage Protection:** 5.5V clamp on sensor inputs
- **ESD Protection:** On all external interfaces
- **Battery Protection:** Overcharge/over-discharge circuits

---

##  Next Steps

1. **Assemble hardware** according to pin mappings
2. **Flash firmware** using Simplicity Studio
3. **Run self-test** to verify all components
4. **Calibrate sensors** for accurate readings
5. **Deploy in test environment** for validation

For questions or issues, refer to the main documentation or create an issue in the GitHub repository.
```

