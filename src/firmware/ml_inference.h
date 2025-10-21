/**
 * KrishiRakshak - ML Inference Header
 * Machine Learning API for Edge AI on EFR32MG26
 */

#ifndef ML_INFERENCE_H
#define ML_INFERENCE_H

#include <stdint.h>
#include <stdbool.h>

// Model configuration
#define ML_MODEL_VERSION         "1.0-edge"
#define ML_MODEL_SIZE            (8.8 * 1024)  // 8.8 KB

// Output class definitions
typedef enum {
    CLASS_NORMAL = 0,
    CLASS_WATER_STRESS = 1,
    CLASS_PEST_RISK = 2,
    CLASS_UNKNOWN = 255
} ml_class_t;

// Sensor data structure
typedef struct {
    float soil_moisture;     // 0-100%
    float temperature;       // Celsius
    float humidity;         // 0-100%
    float audio_energy;     // 0-1 normalized
    uint32_t timestamp;     // milliseconds
} sensor_data_t;

// ML inference result
typedef struct {
    ml_class_t predicted_class;
    float confidence;           // 0-1
    float raw_scores[3];       // Scores for all classes
    uint32_t inference_time_us; // Inference time in microseconds
} ml_result_t;

// Model information
typedef struct {
    uint32_t input_size;
    uint32_t output_size;
    uint32_t arena_used;
    uint32_t arena_size;
    bool is_quantized;
} ml_model_info_t;

// Function prototypes

/**
 * @brief Initialize the ML model and TensorFlow Lite Micro
 * @return true if successful, false otherwise
 */
bool ml_model_init(void);

/**
 * @brief Run ML inference on sensor data
 * @param sensor_data Pointer to sensor data structure
 * @param result Pointer to result structure to fill
 * @return true if inference successful, false otherwise
 */
bool run_ml_inference(const sensor_data_t* sensor_data, ml_result_t* result);

/**
 * @brief Get model information and memory usage
 * @param info Pointer to model info structure to fill
 */
void ml_model_get_info(ml_model_info_t* info);

/**
 * @brief Get class name as string
 * @param class_id Class identifier
 * @return String representation of class
 */
const char* ml_get_class_name(ml_class_t class_id);

/**
 * @brief Get current time in microseconds (for performance measurement)
 * @return Current time in microseconds
 */
uint32_t get_current_time_us(void);

/**
 * @brief Deinitialize ML model and free resources
 */
void ml_model_deinit(void);

#endif // ML_INFERENCE_H
