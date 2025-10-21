/**
 * Machine Learning Inference Engine
 * Optimized for EFR32MG26 MVP (Matrix Vector Processor)
 */

#include "ml_inference.h"
#include "arm_nnfunctions.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

// Model-specific includes (generated from TensorFlow conversion)
#include "krishirakshak_model.h"

// TFLite Micro components
static const tflite::Model* model = nullptr;
static tflite::MicroInterpreter* interpreter = nullptr;
static TfLiteTensor* input_tensor = nullptr;
static TfLiteTensor* output_tensor = nullptr;

// Tensor arena (adjust size based on model requirements)
constexpr int kTensorArenaSize = 20 * 1024;  // 20KB for our model
static uint8_t tensor_arena[kTensorArenaSize] __attribute__((aligned(16)));

// Model quantization parameters
static float input_scale = 0.0f;
static int32_t input_zero_point = 0;
static float output_scale = 0.0f;
static int32_t output_zero_point = 0;

/**
 * Initialize ML model
 */
bool ml_model_init(void) {
  printf("Initializing ML model...\n");
  
  // Load model
  model = tflite::GetModel(krishirakshak_model_tflite);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    printf(" Model version mismatch\n");
    return false;
  }
  
  // Register operations (only those used by our model)
  static tflite::MicroMutableOpResolver<4> resolver;
  resolver.AddFullyConnected();
  resolver.AddSoftmax();
  resolver.AddRelu();
  resolver.AddQuantize();
  
  // Build interpreter
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;
  
  // Allocate tensors
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    printf(" Tensor allocation failed\n");
    return false;
  }
  
  // Get input and output tensors
  input_tensor = interpreter->input(0);
  output_tensor = interpreter->output(0);
  
  // Get quantization parameters
  input_scale = input_tensor->params.scale;
  input_zero_point = input_tensor->params.zero_point;
  output_scale = output_tensor->params.scale;
  output_zero_point = output_tensor->params.zero_point;
  
  printf(" ML model initialized successfully\n");
  printf("   Input: %d bytes, dims: ", input_tensor->bytes);
  for (int i = 0; i < input_tensor->dims->size; i++) {
    printf("%d ", input_tensor->dims->data[i]);
  }
  printf("\n");
  
  printf("   Output: %d bytes, dims: ", output_tensor->bytes);
  for (int i = 0; i < output_tensor->dims->size; i++) {
    printf("%d ", output_tensor->dims->data[i]);
  }
  printf("\n");
  
  return true;
}

/**
 * Run ML inference on sensor data
 */
bool run_ml_inference(const sensor_data_t* sensor_data, ml_result_t* result) {
  if (!interpreter || !input_tensor || !output_tensor) {
    printf(" ML model not initialized\n");
    return false;
  }
  
  // Preprocess sensor data into features
  float features[4];
  features[0] = sensor_data->soil_moisture / 100.0f;        // Normalize 0-100% to 0-1
  features[1] = (sensor_data->temperature - 10.0f) / 40.0f; // Normalize 10-50°C to 0-1
  features[2] = sensor_data->humidity / 100.0f;             // Normalize 0-100% to 0-1
  features[3] = sensor_data->audio_energy;                  // Already 0-1
  
  // Quantize input features to int8
  int8_t quantized_input[4];
  for (int i = 0; i < 4; i++) {
    quantized_input[i] = (int8_t)(features[i] / input_scale + input_zero_point);
  }
  
  // Copy quantized input to tensor
  memcpy(input_tensor->data.int8, quantized_input, sizeof(quantized_input));
  
  // Run inference with timing
  uint32_t start_time = get_current_time_us();
  TfLiteStatus invoke_status = interpreter->Invoke();
  uint32_t inference_time = get_current_time_us() - start_time;
  
  if (invoke_status != kTfLiteOk) {
    printf(" ML inference failed\n");
    return false;
  }
  
  // Process output
  int8_t* output_data = output_tensor->data.int8;
  
  // Find highest confidence class
  int8_t max_score = output_data[0];
  int max_index = 0;
  
  for (int i = 1; i < 3; i++) {
    if (output_data[i] > max_score) {
      max_score = output_data[i];
      max_index = i;
    }
  }
  
  // Dequantize confidence score
  float confidence = (max_score - output_zero_point) * output_scale;
  
  // Fill result structure
  result->predicted_class = max_index;
  result->confidence = confidence;
  result->inference_time_us = inference_time;
  
  // Copy raw scores for debugging
  for (int i = 0; i < 3; i++) {
    result->raw_scores[i] = (output_data[i] - output_zero_point) * output_scale;
  }
  
  printf(" Inference time: %lu μs\n", inference_time);
  printf(" Raw scores: Normal=%.3f, WaterStress=%.3f, PestRisk=%.3f\n",
         result->raw_scores[0], result->raw_scores[1], result->raw_scores[2]);
  
  return true;
}

/**
 * Get model information
 */
void ml_model_get_info(ml_model_info_t* info) {
  if (model && interpreter) {
    info->input_size = input_tensor->bytes;
    info->output_size = output_tensor->bytes;
    info->arena_used = interpreter->arena_used_bytes();
    info->arena_size = kTensorArenaSize;
    info->is_quantized = (input_tensor->type == kTfLiteInt8);
  }
}

/**
 * Utility function to get current time in microseconds
 */
uint32_t get_current_time_us(void) {
  // Implementation depends on your hardware timer
  // This is a placeholder - use CMSIS or Silicon Labs timer
  return system_tick * 1000; // Convert ms to μs (simplified)
}
