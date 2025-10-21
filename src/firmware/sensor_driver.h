/**
 * KrishiRakshak - Sensor Driver Header
 * Hardware Abstraction Layer for Agricultural Sensors
 */

#ifndef SENSOR_DRIVER_H
#define SENSOR_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

// Sensor configuration
#define SOIL_SENSOR_ADC_CHANNEL      adcPosSelAPORT3XCH0  // PA0
#define TEMP_SENSOR_I2C_ADDRESS      0x44                 // SHT31 default
#define AUDIO_SAMPLE_RATE            16000                // 16kHz
#define AUDIO_BUFFER_SIZE            256                  // 16ms at 16kHz

// Sensor error codes
typedef enum {
    SENSOR_OK = 0,
    SENSOR_ERROR_ADC,
    SENSOR_ERROR_I2C,
    SENSOR_ERROR_TIMEOUT,
    SENSOR_ERROR_CALIBRATION
} sensor_status_t;

// Audio processing structure
typedef struct {
    float* samples;
    uint32_t sample_count;
    float spectral_energy;
    float peak_frequency;
} audio_frame_t;

// Function prototypes

/**
 * @brief Initialize all sensors
 * @return true if all sensors initialized successfully
 */
bool sensor_init(void);

/**
 * @brief Read soil moisture from capacitive sensor
 * @return Soil moisture percentage (0-100%), or -1 on error
 */
float read_soil_moisture(void);

/**
 * @brief Read temperature from SHT31 sensor
 * @return Temperature in Celsius, or -100 on error
 */
float read_temperature(void);

/**
 * @brief Read humidity from SHT31 sensor
 * @return Relative humidity percentage (0-100%), or -1 on error
 */
float read_humidity(void);

/**
 * @brief Process audio samples for pest detection
 * @return Audio energy (0-1 normalized), or -1 on error
 */
float process_audio_samples(void);

/**
 * @brief Read battery voltage
 * @return Battery voltage in volts, or -1 on error
 */
float read_battery_voltage(void);

/**
 * @brief Put sensors into low-power sleep mode
 */
void sensor_sleep(void);

/**
 * @brief Wake sensors from sleep mode
 */
void sensor_wake(void);

/**
 * @brief Calibrate soil moisture sensor
 * @param dry_value ADC value for dry conditions
 * @param wet_value ADC value for wet conditions
 * @return true if calibration successful
 */
bool calibrate_soil_sensor(uint16_t dry_value, uint16_t wet_value);

/**
 * @brief Validate sensor readings for plausibility
 * @param moisture Soil moisture reading
 * @param temp Temperature reading
 * @param humidity Humidity reading
 * @return true if readings are plausible
 */
bool validate_sensor_readings(float moisture, float temp, float humidity);

/**
 * @brief Get sensor status and diagnostics
 * @return Sensor status code
 */
sensor_status_t get_sensor_status(void);

/**
 * @brief Perform FFT on audio samples for frequency analysis
 * @param audio Pointer to audio frame
 * @param magnitudes Output buffer for FFT magnitudes
 * @param num_bins Number of frequency bins
 */
void perform_audio_fft(const audio_frame_t* audio, float* magnitudes, uint32_t num_bins);

#endif // SENSOR_DRIVER_H
