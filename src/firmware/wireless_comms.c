/**
 * Wireless Communication Module
 * SiWx917 for BLE/Wi-Fi alerts
 */

#include "wireless_comms.h"
#include "sl_wifi.h"
#include "sl_net.h"
#include "sl_net_wifi_types.h"
#include "sl_net_default_values.h"

// Wireless configuration
static sl_net_wifi_credentials_t wifi_credentials = {
  .ssid = WIFI_SSID,
  .ssid_length = sizeof(WIFI_SSID) - 1,
  .type = SL_NET_WIFI_PSK,
  .credentials = {
    .psk = WIFI_PASSWORD,
    .psk_length = sizeof(WIFI_PASSWORD) - 1
  }
};

static bool wireless_connected = false;
static uint32_t last_alert_time = 0;

/**
 * Initialize wireless communication
 */
bool wireless_init(void) {
  printf(" Initializing wireless communication...\n");
  
  sl_status_t status;
  
  // Initialize Wi-Fi
  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &sl_net_default_wifi_configuration);
  if (status != SL_STATUS_OK) {
    printf(" Wi-Fi initialization failed: 0x%lx\n", status);
    return false;
  }
  
  // Bring up the Wi-Fi interface
  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
  if (status != SL_STATUS_OK) {
    printf(" Wi-Fi interface up failed: 0x%lx\n", status);
    return false;
  }
  
  printf(" Wireless initialized successfully\n");
  return true;
}

/**
 * Connect to Wi-Fi network
 */
bool wireless_connect(void) {
  printf(" Connecting to Wi-Fi: %s\n", WIFI_SSID);
  
  sl_status_t status = sl_net_wifi_add_profile(&wifi_credentials, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
  if (status != SL_STATUS_OK) {
    printf(" Failed to add Wi-Fi profile: 0x%lx\n", status);
    return false;
  }
  
  // Wait for connection
  for (int retry = 0; retry < 10; retry++) {
    sl_net_wifi_client_profile_status_t connection_status;
    status = sl_net_wifi_get_profile_status(SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID, &connection_status);
    
    if (status == SL_STATUS_OK && connection_status.state == SL_NET_WIFI_CLIENT_CONNECTED) {
      wireless_connected = true;
      printf(" Wi-Fi connected successfully\n");
      
      // Print connection info
      sl_net_wifi_client_profile_info_t profile_info;
      sl_net_wifi_get_profile_info(SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID, &profile_info);
      printf("   IP: %s, RSSI: %ddBm\n", profile_info.ip_address, profile_info.rssi_val);
      
      return true;
    }
    
    printf(" Waiting for connection... (%d/10)\n", retry + 1);
    delay(1000);
  }
  
  printf(" Wi-Fi connection timeout\n");
  return false;
}

/**
 * Send wireless alert
 */
bool send_wireless_alert(alert_type_t alert_type, const sensor_data_t* sensor_data, float confidence) {
  // Rate limiting - don't send alerts too frequently
  if ((system_tick - last_alert_time) < MIN_ALERT_INTERVAL_MS) {
    printf("  Alert rate limited, skipping...\n");
    return false;
  }
  
  // Ensure we're connected
  if (!wireless_connected && !wireless_connect()) {
    printf(" Cannot send alert - no wireless connection\n");
    return false;
  }
  
  // Create alert message
  alert_message_t alert;
  alert.device_id = DEVICE_ID;
  alert.timestamp = system_tick;
  alert.alert_type = alert_type;
  alert.confidence = (uint8_t)(confidence * 100);
  
  if (sensor_data) {
    alert.sensor_data = *sensor_data;
  }
  
  // Convert to JSON for transmission
  char json_buffer[256];
  int json_length = format_alert_json(&alert, json_buffer, sizeof(json_buffer));
  
  if (json_length <= 0) {
    printf(" Failed to format alert JSON\n");
    return false;
  }
  
  printf(" Sending alert: %s\n", get_alert_type_string(alert_type));
  
  // Send via HTTP POST (or MQTT in production)
  bool success = send_http_alert(json_buffer, json_length);
  
  if (success) {
    last_alert_time = system_tick;
    printf(" Alert sent successfully\n");
  } else {
    printf(" Failed to send alert\n");
  }
  
  return success;
}

/**
 * Format alert as JSON
 */
int format_alert_json(const alert_message_t* alert, char* buffer, size_t buffer_size) {
  return snprintf(buffer, buffer_size,
    "{\n"
    "  \"device_id\": \"%s\",\n"
    "  \"timestamp\": %lu,\n"
    "  \"alert_type\": \"%s\",\n"
    "  \"confidence\": %d,\n"
    "  \"sensor_data\": {\n"
    "    \"moisture\": %.1f,\n"
    "    \"temperature\": %.1f,\n"
    "    \"humidity\": %.1f,\n"
    "    \"audio\": %.2f\n"
    "  },\n"
    "  \"recommendation\": \"%s\"\n"
    "}",
    alert->device_id,
    alert->timestamp,
    get_alert_type_string(alert->alert_type),
    alert->confidence,
    alert->sensor_data.soil_moisture,
    alert->sensor_data.temperature,
    alert->sensor_data.humidity,
    alert->sensor_data.audio_energy,
    get_alert_recommendation(alert->alert_type)
  );
}

/**
 * Send HTTP alert to cloud/dashboard
 */
bool send_http_alert(const char* json_data, int data_length) {
  sl_status_t status;
  sl_net_http_client_t http_client = {0};
  sl_net_http_client_config_t http_config = {0};
  
  // Configure HTTP client
  http_config.server_name = CLOUD_SERVER;
  http_config.server_port = CLOUD_PORT;
  http_config.timeout_ms = 10000;
  
  status = sl_net_http_client_init(&http_client, &http_config);
  if (status != SL_STATUS_OK) {
    printf(" HTTP client init failed: 0x%lx\n", status);
    return false;
  }
  
  // Send POST request
  sl_net_http_client_response_t response = {0};
  status = sl_net_http_client_post(&http_client, CLOUD_ALERT_ENDPOINT, 
                                  "application/json", json_data, data_length, &response);
  
  sl_net_http_client_deinit(&http_client);
  
  if (status == SL_STATUS_OK && response.status_code == 200) {
    return true;
  } else {
    printf(" HTTP POST failed - Status: 0x%lx, Code: %d\n", status, response.status_code);
    return false;
  }
}

/**
 * Get alert type as string
 */
const char* get_alert_type_string(alert_type_t alert_type) {
  switch (alert_type) {
    case ALERT_WATER_STRESS: return "water_stress";
    case ALERT_PEST_RISK: return "pest_risk";
    case ALERT_LOW_BATTERY: return "low_battery";
    default: return "unknown";
  }
}

/**
 * Get recommendation for alert type
 */
const char* get_alert_recommendation(alert_type_t alert_type) {
  switch (alert_type) {
    case ALERT_WATER_STRESS: return "Initiate irrigation in affected area";
    case ALERT_PEST_RISK: return "Inspect crops for pest activity and consider treatment";
    case ALERT_LOW_BATTERY: return "Check solar panel and charging system";
    default: return "Monitor situation";
  }
}

/**
 * Check wireless connection status
 */
bool is_wireless_connected(void) {
  return wireless_connected;
}
