/**
 * KrishiRakshak - Wireless Communication Header
 * SiWx917 BLE/Wi-Fi Interface for Agricultural Alerts
 */

#ifndef WIRELESS_COMMS_H
#define WIRELESS_COMMS_H

#include <stdint.h>
#include <stdbool.h>
#include "ml_inference.h"

// Wireless configuration
#define WIFI_SSID               "KrishiRakshak_AP"
#define WIFI_PASSWORD           "farm123456"
#define DEVICE_ID               "KR_001"
#define CLOUD_SERVER            "api.krishirakshak.com"
#define CLOUD_PORT              443
#define CLOUD_ALERT_ENDPOINT    "/api/v1/alerts"

// Alert configuration
#define MIN_ALERT_INTERVAL_MS   300000  // 5 minutes between alerts
#define BATTERY_LOW_THRESHOLD   3.3f    // 3.3V low battery threshold

// Alert types
typedef enum {
    ALERT_WATER_STRESS = 0,
    ALERT_PEST_RISK = 1,
    ALERT_LOW_BATTERY = 2,
    ALERT_SYSTEM_ERROR = 3
} alert_type_t;

// Alert message structure
typedef struct {
    char device_id[16];
    uint32_t timestamp;
    alert_type_t alert_type;
    uint8_t confidence;      // 0-100%
    sensor_data_t sensor_data;
} alert_message_t;

// Function prototypes

/**
 * @brief Initialize wireless communication module
 * @return true if successful, false otherwise
 */
bool wireless_init(void);

/**
 * @brief Connect to Wi-Fi network
 * @return true if connected successfully, false otherwise
 */
bool wireless_connect(void);

/**
 * @brief Send wireless alert to cloud/dashboard
 * @param alert_type Type of alert to send
 * @param sensor_data Pointer to sensor data (can be NULL for system alerts)
 * @param confidence Confidence level (0-1)
 * @return true if alert sent successfully, false otherwise
 */
bool send_wireless_alert(alert_type_t alert_type, const sensor_data_t* sensor_data, float confidence);

/**
 * @brief Check if wireless is connected
 * @return true if connected, false otherwise
 */
bool is_wireless_connected(void);

/**
 * @brief Disconnect from wireless network
 */
void wireless_disconnect(void);

/**
 * @brief Get wireless connection status information
 * @param rssi Pointer to store RSSI value (optional)
 * @param ip_address Buffer to store IP address (optional)
 * @return true if status retrieved successfully
 */
bool wireless_get_status(int8_t* rssi, char* ip_address);

/**
 * @brief Format alert as JSON string
 * @param alert Pointer to alert message
 * @param buffer Output buffer for JSON
 * @param buffer_size Size of output buffer
 * @return Length of JSON string, or -1 on error
 */
int format_alert_json(const alert_message_t* alert, char* buffer, size_t buffer_size);

/**
 * @brief Get string representation of alert type
 * @param alert_type Alert type
 * @return String representation
 */
const char* get_alert_type_string(alert_type_t alert_type);

/**
 * @brief Get recommendation for alert type
 * @param alert_type Alert type
 * @return Recommendation string
 */
const char* get_alert_recommendation(alert_type_t alert_type);

#endif // WIRELESS_COMMS_H
