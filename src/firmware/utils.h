/**
 * KrishiRakshak - Utility Functions Header
 * Common utilities and helper functions
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// Utility macros
#define ARRAY_SIZE(x)           (sizeof(x) / sizeof((x)[0]))
#define MIN(a, b)               ((a) < (b) ? (a) : (b))
#define MAX(a, b)               ((a) > (b) ? (a) : (b))
#define CLAMP(x, min, max)      ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

// Bit manipulation macros
#define BIT(n)                  (1UL << (n))
#define SET_BIT(reg, bit)       ((reg) |= (bit))
#define CLEAR_BIT(reg, bit)     ((reg) &= ~(bit))
#define TOGGLE_BIT(reg, bit)    ((reg) ^= (bit))
#define CHECK_BIT(reg, bit)     ((reg) & (bit))

// Circular buffer implementation
typedef struct {
    float* buffer;
    uint32_t size;
    uint32_t head;
    uint32_t tail;
    uint32_t count;
} circular_buffer_t;

// Function prototypes

/**
 * @brief Initialize circular buffer
 * @param cb Pointer to circular buffer structure
 * @param buffer Pointer to data buffer
 * @param size Size of buffer
 */
void circular_buffer_init(circular_buffer_t* cb, float* buffer, uint32_t size);

/**
 * @brief Push value to circular buffer
 * @param cb Pointer to circular buffer
 * @param value Value to push
 * @return true if successful, false if buffer full
 */
bool circular_buffer_push(circular_buffer_t* cb, float value);

/**
 * @brief Pop value from circular buffer
 * @param cb Pointer to circular buffer
 * @param value Pointer to store popped value
 * @return true if successful, false if buffer empty
 */
bool circular_buffer_pop(circular_buffer_t* cb, float* value);

/**
 * @brief Check if circular buffer is empty
 * @param cb Pointer to circular buffer
 * @return true if empty, false otherwise
 */
bool circular_buffer_empty(const circular_buffer_t* cb);

/**
 * @brief Check if circular buffer is full
 * @param cb Pointer to circular buffer
 * @return true if full, false otherwise
 */
bool circular_buffer_full(const circular_buffer_t* cb);

/**
 * @brief Calculate mean of values in circular buffer
 * @param cb Pointer to circular buffer
 * @return Mean value, or 0 if buffer empty
 */
float circular_buffer_mean(const circular_buffer_t* cb);

/**
 * @brief Calculate standard deviation of values in circular buffer
 * @param cb Pointer to circular buffer
 * @return Standard deviation, or 0 if buffer empty
 */
float circular_buffer_stddev(const circular_buffer_t* cb);

/**
 * @brief Simple moving average filter
 * @param values Array of values
 * @param count Number of values
 * @param window_size Moving average window size
 * @return Moving average
 */
float moving_average(const float* values, uint32_t count, uint32_t window_size);

/**
 * @brief Exponential moving average filter
 * @param current_value Current value
 * @param previous_ema Previous EMA value
 * @param alpha Smoothing factor (0-1)
 * @return Exponential moving average
 */
float exponential_moving_average(float current_value, float previous_ema, float alpha);

/**
 * @brief Normalize value from one range to another
 * @param value Input value
 * @param from_min Source range minimum
 * @param from_max Source range maximum
 * @param to_min Target range minimum
 * @param to_max Target range maximum
 * @return Normalized value
 */
float normalize_value(float value, float from_min, float from_max, float to_min, float to_max);

/**
 * @brief Convert Celsius to Fahrenheit
 * @param celsius Temperature in Celsius
 * @return Temperature in Fahrenheit
 */
float celsius_to_fahrenheit(float celsius);

/**
 * @brief Convert Fahrenheit to Celsius
 * @param fahrenheit Temperature in Fahrenheit
 * @return Temperature in Celsius
 */
float fahrenheit_to_celsius(float fahrenheit);

/**
 * @brief Calculate battery percentage from voltage
 * @param voltage Battery voltage
 * @param min_voltage Minimum operating voltage
 * @param max_voltage Maximum charging voltage
 * @return Battery percentage (0-100)
 */
uint8_t battery_voltage_to_percentage(float voltage, float min_voltage, float max_voltage);

/**
 * @brief Simple delay function
 * @param ms Delay in milliseconds
 */
void delay_ms(uint32_t ms);

/**
 * @brief Get system uptime in milliseconds
 * @return Uptime in milliseconds
 */
uint32_t get_uptime_ms(void);

/**
 * @brief Calculate checksum for data integrity
 * @param data Pointer to data
 * @param length Data length in bytes
 * @return Checksum value
 */
uint16_t calculate_checksum(const uint8_t* data, uint32_t length);

/**
 * @brief Verify checksum for data integrity
 * @param data Pointer to data
 * @param length Data length in bytes
 * @param checksum Expected checksum
 * @return true if checksum matches, false otherwise
 */
bool verify_checksum(const uint8_t* data, uint32_t length, uint16_t checksum);

#endif // UTILS_H
