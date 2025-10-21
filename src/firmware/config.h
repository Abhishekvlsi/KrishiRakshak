/**
 * KrishiRakshak - System Configuration Header
 * Centralized configuration for all system parameters
 */

#ifndef CONFIG_H
#define CONFIG_H

// System version
#define FIRMWARE_VERSION        "1.0.0"
#define HARDWARE_VERSION        "RevA"

// Debug configuration
#define DEBUG_ENABLED           1
#define SERIAL_BAUDRATE         115200

#if DEBUG_ENABLED
    #define DEBUG_PRINT(fmt, ...)   printf(fmt, ##__VA_ARGS__)
    #define DEBUG_ERROR(fmt, ...)   printf("ERROR: " fmt, ##__VA_ARGS__)
    #define DEBUG_WARN(fmt, ...)    printf("WARN: " fmt, ##__VA_ARGS__)
    #define DEBUG_INFO(fmt, ...)    printf("INFO: " fmt, ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...)
    #define DEBUG_ERROR(fmt, ...)
    #define DEBUG_WARN(fmt, ...)
    #define DEBUG_INFO(fmt, ...)
#endif

// Power management configuration
#define POWER_SLEEP_DURATION_MS     30000    // 30 seconds sleep
#define POWER_DEEP_SLEEP_DURATION_MS 300000  // 5 minutes deep sleep
#define POWER_BATTERY_CRITICAL       3.2f    // Critical battery level
#define POWER_BATTERY_LOW            3.5f    // Low battery level

// ML configuration
#define ML_CONFIDENCE_THRESHOLD     0.7f     // 70% confidence for alerts
#define ML_MAX_INFERENCE_TIME_MS    50       // Maximum allowed inference time

// Sensor configuration
#define SENSOR_READ_INTERVAL_MS     30000    // 30 seconds between readings
#define SENSOR_CALIBRATION_INTERVAL 86400000 // 24 hours calibration check

// Wireless configuration
#define WIFI_CONNECT_TIMEOUT_MS     10000    // 10 seconds connection timeout
#define WIFI_RETRY_INTERVAL_MS      60000    // 1 minute between retries
#define CLOUD_SYNC_INTERVAL_MS      3600000  // 1 hour cloud sync

// Alert configuration
#define ALERT_MAX_RETRIES           3
#define ALERT_RETRY_INTERVAL_MS     5000     // 5 seconds between retries

// Memory configuration
#define TENSOR_ARENA_SIZE           (20 * 1024)  // 20KB for TFLite
#define SENSOR_BUFFER_SIZE          (2 * 1024)   // 2KB for sensor data
#define WIRELESS_BUFFER_SIZE        (1 * 1024)   // 1KB for wireless data

// MVP optimization configuration
#define MVP_OPTIMIZATION_ENABLED    1
#define CMSIS_NN_ENABLED            1

#if MVP_OPTIMIZATION_ENABLED
    #define ML_USE_MVP              1
    #define ML_MEMORY_ALIGNMENT     16
#else
    #define ML_USE_MVP              0
    #define ML_MEMORY_ALIGNMENT     4
#endif

// System thresholds
#define TEMPERATURE_HIGH_THRESHOLD  40.0f    // High temperature warning
#define TEMPERATURE_LOW_THRESHOLD   5.0f     // Low temperature warning
#define HUMIDITY_HIGH_THRESHOLD     90.0f    // High humidity warning
#define HUMIDITY_LOW_THRESHOLD      20.0f    // Low humidity warning
#define SOIL_MOISTURE_LOW_THRESHOLD 25.0f    // Low soil moisture warning

// Performance monitoring
#define PERFORMANCE_SAMPLES         100      // Number of samples to track
#define PERFORMANCE_REPORT_INTERVAL 3600000  // 1 hour performance report

#endif // CONFIG_H
