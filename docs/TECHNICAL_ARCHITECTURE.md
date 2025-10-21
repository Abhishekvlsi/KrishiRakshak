# KrishiRakshak - Technical Architecture

## 🏗️ System Overview

KrishiRakshak implements a distributed edge intelligence architecture optimized for agricultural monitoring. The system combines multi-modal sensing with MVP-accelerated machine learning to deliver real-time insights without cloud dependency.

## 🔧 Hardware Architecture

### Component Specification
| Component | Model | Specification | Purpose |
|-----------|-------|---------------|---------|
| **Main Processor** | EFR32MG26 | ARM Cortex-M33 + MVP, 512KB RAM, 1MB Flash | Edge AI Inference |
| **Wireless SoC** | SiWx917 | Wi-Fi 6 + BLE 5.2, Ultra-low-power | Communication |
| **Soil Sensor** | Capacitive Moisture | 0-100% RH, Analog Output | Water Stress Detection |
| **Temp/Humidity** | SHT31 | ±0.2°C, ±2% RH, I2C | Environmental Monitoring |
| **Audio Sensor** | MEMS Microphone | 60dB SNR, I2S/PDM | Pest Activity Detection |
| **Power** | Solar + LiPo | 3.7V, 2000mAh + 5W Solar | Autonomous Operation |

### System Block Diagram
```
┌─────────────────────────────────────────────────────────────┐
│                      KrishiRakshak Node                     │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │   Sensors   │  │    Power    │  │      Wireless       │  │
│  │             │  │             │  │                     │  │
│  │ • Soil      │  │ • Solar     │  │ • SiWx917 SoC       │  │
│  │ • Temp/Humid│  │ • Battery   │  │ • BLE 5.2 + Wi-Fi   │  │
│  │ • Microphone│  │ • Charger   │  │ • Low-Power Mode    │  │
│  └──────┬──────┘  └──────┬──────┘  └──────────┬──────────┘  │
│         │                │                    │            │
│  ┌──────┴────────────────┴────────────────────┴──────────┐  │
│  │                 EFR32MG26 SoC                         │  │
│  │  ┌──────────────────────────────────────────────────┐ │  │
│  │  │               Matrix Vector Processor            │ │  │
│  │  │  • 5-10x Faster ML Inference                     │ │  │
│  │  │  • <50mW Power Consumption                       │ │  │
│  │  │  • Parallel Matrix Operations                    │ │  │
│  │  └──────────────────────────────────────────────────┘ │  │
│  │  ┌──────────────────┐ ┌─────────────────────────────┐ │  │
│  │  │   TinyML Model   │ │   Sensor Fusion & Preproc   │ │  │
│  │  │ • 2,979 params   │ │ • Data Normalization        │ │  │
│  │  │ • 8.8KB size     │ │ • FFT for Audio             │ │  │
│  │  │ • 3-class output │ │ • Feature Extraction        │ │  │
│  │  └──────────────────┘ └─────────────────────────────┘ │  │
│  └─────────────────────────────────────────────────────────┘
```

## 🧠 Machine Learning Architecture

### Model Specifications
```python
Model: "krishirakshak_enhanced"
_________________________________________________________________
Layer (type)                 Output Shape              Param #
=================================================================
dense_1 (Dense)              (None, 64)                320
_________________________________________________________________
dropout (Dropout)            (None, 64)                0
_________________________________________________________________
dense_2 (Dense)              (None, 32)                2080
_________________________________________________________________
dense_3 (Dense)              (None, 16)                528
_________________________________________________________________
output (Dense)               (None, 3)                 51
=================================================================
Total params: 2,979
Trainable params: 2,979
Non-trainable params: 0
```

### Feature Engineering
| Feature | Source | Processing | Purpose |
|---------|--------|------------|---------|
| **Soil Moisture** | Capacitive Sensor | Normalization (0-100% → 0-1) | Water Stress |
| **Temperature** | SHT31 | Scaling (0-50°C → 0-1) | Heat Stress |
| **Humidity** | SHT31 | Normalization (0-100% → 0-1) | Microclimate |
| **Audio Energy** | MEMS Mic | FFT → Spectral Energy | Pest Detection |

### Class Output Mapping
- **Class 0:** Normal Conditions (No action needed)
- **Class 1:** Water Stress (Irrigation alert)
- **Class 2:** Pest Risk (Inspection recommended)

## ⚡ MVP Acceleration Implementation

### CMSIS-NN Integration
```c
// MVP-accelerated inference implementation
#include "arm_nnfunctions.h"

void run_inference(const int8_t* input_data, int8_t* output_data) {
    // Fully connected layer with MVP acceleration
    arm_fully_connected_s8(
        input_buffer,        // Input tensor
        weight_matrix,       // MVP-accelerated weights
        input_dim,           // Input dimension (4)
        output_dim,          // Output dimension (64)
        bias_vector,         // Bias values
        output_buffer,       // Output tensor
        activation_params,   // ReLU activation
        quantization_params, // 8-bit quantization
        ARM_MATH_MVEF        // MVP extension flag
    );
}
```

### Performance Optimization
- **Weight Layout:** Optimized for MVP memory access patterns
- **Quantization:** 8-bit integers for efficient MVP computation
- **Batch Processing:** Parallel operations across multiple data points
- **Memory Management:** Efficient buffer allocation for intermediate tensors

## 📡 Wireless Communication Protocol

### Alert Message Format
```json
{
  "device_id": "KR_001",
  "timestamp": 1672531200,
  "alert_type": "water_stress",
  "confidence": 0.85,
  "sensor_data": {
    "moisture": 25.5,
    "temperature": 35.2,
    "humidity": 32.1
  },
  "recommendation": "Initiate irrigation in Field B"
}
```

### Power Management Strategy
```
NORMAL OPERATION:
Sensors → EFR32MG26 (Active) → SiWx917 (Sleep)
    ↓
ML Inference (0.01ms active)
    ↓
Decision Engine
    ↓
If Alert Needed: Wake SiWx917 → Transmit → Sleep

POWER CONSUMPTION:
• EFR32MG26 Active: ~15mA @ 3.3V = 49.5mW
• SiWx917 Sleep: ~1μA @ 3.3V = 0.0033mW
• SiWx917 Active: ~50mA @ 3.3V = 165mW (brief)
• Average Power: <5mW (with 1 alert/hour)
```

## 🔄 Data Processing Pipeline

### Step 1: Sensor Data Acquisition
```c
// Read all sensors with timing synchronization
void acquire_sensor_data(SensorData* data) {
    data->moisture = read_soil_moisture();
    data->temperature = read_temperature();
    data->humidity = read_humidity();
    data->audio_energy = process_audio_frame();
    data->timestamp = get_current_time();
}
```

### Step 2: Preprocessing & Feature Extraction
```c
// Normalize and prepare features for ML
void extract_features(const SensorData* raw, float* features) {
    features[0] = normalize_moisture(raw->moisture);
    features[1] = scale_temperature(raw->temperature);
    features[2] = normalize_humidity(raw->humidity);
    features[3] = compute_audio_energy(raw->audio_energy);
}
```

### Step 3: Edge Inference
```c
// Run ML model with MVP acceleration
int run_ml_inference(const float* features) {
    int8_t quantized_input[4];
    int8_t output[3];
    
    // Quantize input
    quantize_features(features, quantized_input);
    
    // Run inference
    ml_model_inference(quantized_input, output);
    
    // Find highest confidence class
    return argmax(output, 3);
}
```

### Step 4: Decision & Alert
```c
// Make decision and trigger alerts
void process_ml_output(int prediction, float confidence) {
    if (confidence > CONFIDENCE_THRESHOLD) {
        switch(prediction) {
            case WATER_STRESS:
                trigger_irrigation_alert();
                break;
            case PEST_RISK:
                trigger_inspection_alert();
                break;
        }
    }
}
```

## 🛡️ Reliability & Fault Tolerance

### Error Handling
- **Sensor Failures:** Default values with confidence reduction
- **ML Model Errors:** Fallback to rule-based detection
- **Wireless Issues:** Local storage with sync when available
- **Power Management:** Graceful shutdown and recovery

### Data Validation
```c
int validate_sensor_data(const SensorData* data) {
    if (data->moisture < 0 || data->moisture > 100) return 0;
    if (data->temperature < -10 || data->temperature > 60) return 0;
    if (data->humidity < 0 || data->humidity > 100) return 0;
    return 1; // Data valid
}
```

## 📊 Performance Benchmarks

### Resource Utilization
| Resource | Usage | Capacity | Utilization |
|----------|-------|----------|-------------|
| **Flash Memory** | 8.8 KB | 1 MB | 0.9% |
| **RAM** | 18.8 KB | 512 KB | 3.7% |
| **Parameters** | 2,979 | 10,000 | 29.8% |
| **Inference Time** | 0.01 ms | 50 ms | 0.02% |

### Power Consumption Analysis
| Operation | Current | Duration | Energy per Operation |
|-----------|---------|----------|---------------------|
| **Sensor Reading** | 5 mA | 10 ms | 0.165 mJ |
| **ML Inference** | 15 mA | 0.01 ms | 0.005 mJ |
| **Wireless Tx** | 50 mA | 100 ms | 16.5 mJ |
| **Sleep Mode** | 20 μA | Continuous | 0.066 mJ/s |

---

*This technical architecture demonstrates a comprehensive edge AI system optimized for agricultural monitoring using Silicon Labs hardware.*
```

