/**
 * KrishiRakshak - Main Firmware
 * Silicon Labs EFR32MG26 + SiWx917
 * Edge AI for Smart Agriculture
 */

#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_adc.h"
#include "sl_i2cspm.h"
#include "ml_inference.h"
#include "wireless_comms.h"
#include "sensor_driver.h"

// System configuration
#define SENSOR_READ_INTERVAL_MS  30000    // 30 seconds between readings
#define ML_CONFIDENCE_THRESHOLD  70       // 70% confidence for alerts
#define BATTERY_CHECK_INTERVAL   3600000  // 1 hour battery check

// Global variables
static volatile uint32_t system_tick = 0;
static sensor_data_t current_sensor_data;
static bool alert_pending = false;

// Function prototypes
static void system_init(void);
static void read_all_sensors(void);
static void process_sensor_data(void);
static void enter_low_power_mode(void);
static void check_battery_level(void);

int main(void) {
  // Chip initialization
  CHIP_Init();
  system_init();
  
  printf(" KrishiRakshak Firmware Started\n");
  printf("Build: %s %s\n", __DATE__, __TIME__);
  printf("Model: %s, Size: %d bytes\n", ML_MODEL_VERSION, ML_MODEL_SIZE);
  
  // Main application loop
  while (1) {
    // Read sensors
    read_all_sensors();
    
    // Process data and run ML inference
    process_sensor_data();
    
    // Check battery periodically
    if (system_tick % BATTERY_CHECK_INTERVAL == 0) {
      check_battery_level();
    }
    
    // Enter low power mode until next reading
    enter_low_power_mode();
    
    system_tick += SENSOR_READ_INTERVAL_MS;
  }
}

/**
 * System initialization
 */
static void system_init(void) {
  // Initialize clock
  CMU_ClockEnable(cmuClock_GPIO, true);
  CMU_ClockEnable(cmuClock_ADC0, true);
  
  // Initialize peripherals
  sensor_init();
  wireless_init();
  ml_model_init();
  
  // Configure LED for status indication
  GPIO_PinModeSet(gpioPortA, 0, gpioModePushPull, 0); // Green LED
  GPIO_PinModeSet(gpioPortA, 1, gpioModePushPull, 0); // Red LED
  
  printf("System initialization complete\n");
}

/**
 * Read all sensors
 */
static void read_all_sensors(void) {
  printf("Reading sensors...\n");
  
  // Read soil moisture (ADC)
  current_sensor_data.soil_moisture = read_soil_moisture();
  
  // Read temperature and humidity (I2C)
  current_sensor_data.temperature = read_temperature();
  current_sensor_data.humidity = read_humidity();
  
  // Read audio for pest detection
  current_sensor_data.audio_energy = process_audio_samples();
  
  // Add timestamp
  current_sensor_data.timestamp = system_tick;
  
  printf(" Sensor Data - Moisture: %.1f%%, Temp: %.1f°C, Humidity: %.1f%%, Audio: %.2f\n",
         current_sensor_data.soil_moisture,
         current_sensor_data.temperature,
         current_sensor_data.humidity,
         current_sensor_data.audio_energy);
}

/**
 * Process sensor data through ML pipeline
 */
static void process_sensor_data(void) {
  ml_result_t ml_result;
  
  // Run ML inference
  if (run_ml_inference(&current_sensor_data, &ml_result)) {
    printf(" ML Inference - Class: %d, Confidence: %.1f%%\n", 
           ml_result.predicted_class, ml_result.confidence * 100);
    
    // Check if we need to send an alert
    if (ml_result.confidence > (ML_CONFIDENCE_THRESHOLD / 100.0)) {
      switch (ml_result.predicted_class) {
        case CLASS_NORMAL:
          // No alert needed for normal conditions
          GPIO_PinOutSet(gpioPortA, 0); // Green LED
          GPIO_PinOutClear(gpioPortA, 1); // Red LED
          break;
          
        case CLASS_WATER_STRESS:
          printf(" Water stress detected! Confidence: %.1f%%\n", ml_result.confidence * 100);
          send_wireless_alert(ALERT_WATER_STRESS, &current_sensor_data, ml_result.confidence);
          GPIO_PinOutClear(gpioPortA, 0); // Green LED
          GPIO_PinOutSet(gpioPortA, 1); // Red LED
          break;
          
        case CLASS_PEST_RISK:
          printf(" Pest risk detected! Confidence: %.1f%%\n", ml_result.confidence * 100);
          send_wireless_alert(ALERT_PEST_RISK, &current_sensor_data, ml_result.confidence);
          GPIO_PinOutClear(gpioPortA, 0); // Green LED
          GPIO_PinOutSet(gpioPortA, 1); // Red LED
          break;
      }
    } else {
      printf("Low confidence prediction, no alert sent\n");
    }
  } else {
    printf(" ML inference failed\n");
  }
}

/**
 * Enter low power mode
 */
static void enter_low_power_mode(void) {
  printf("Entering low power mode for %d ms...\n", SENSOR_READ_INTERVAL_MS);
  
  // Turn off LEDs
  GPIO_PinOutClear(gpioPortA, 0);
  GPIO_PinOutClear(gpioPortA, 1);
  
  // Put sensors to sleep
  sensor_sleep();
  
  // Enter EM2 sleep mode (RTCC running, RAM retention)
  EMU_EnterEM2(true);
  
  // Wait for next reading (using RTCC wakeup)
  // In actual implementation, use RTCC interrupt
  for (volatile int i = 0; i < 1000000; i++); // Simplified delay
}

/**
 * Check battery level
 */
static void check_battery_level(void) {
  float battery_voltage = read_battery_voltage();
  printf("Battery: %.2fV\n", battery_voltage);
  
  if (battery_voltage < BATTERY_LOW_THRESHOLD) {
    printf(" Low battery warning!\n");
    send_wireless_alert(ALERT_LOW_BATTERY, NULL, 0);
  }
}