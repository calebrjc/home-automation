#include "jcfw/platform/wifi.h"

#include "esp_event.h"
#include "esp_wifi.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "jcfw/trace.h"
#include "jcfw/util/assert.h"
#include "jcfw/util/bit.h"

#define STA_TRACE_TAG "JCFW-WIFI-STA"
#define AP_TRACE_TAG  "JCFW-WIFI-AP"

// -------------------------------------------------------------------------------------------------

static const char *S_AUTHMODE_STRINGS[] = {
    "WIFI_AUTH_OPEN",
    "WIFI_AUTH_WEP",
    "WIFI_AUTH_WPA_PSK",
    "WIFI_AUTH_WPA2_PSK",
    "WIFI_AUTH_WPA_WPA2_PSK",
    "WIFI_AUTH_ENTERPRISE/WIFI_AUTH_WPA2_ENTERPRISE",
    "WIFI_AUTH_WPA3_PSK",
    "WIFI_AUTH_WPA2_WPA3_PSK",
    "WIFI_AUTH_WAPI_PSK",
    "WIFI_AUTH_OWE",
    "WIFI_AUTH_WPA3_ENT_192",
    "WIFI_AUTH_WPA3_EXT_PSK",
    "WIFI_AUTH_WPA3_EXT_PSK_MIXED_MODE",
    "WIFI_AUTH_DPP",
};

static const char *S_REASON_STRINGS[] = {
    [1]   = "WIFI_REASON_UNSPECIFIED",
    [2]   = "WIFI_REASON_AUTH_EXPIRE",
    [3]   = "WIFI_REASON_AUTH_LEAVE",
    [4]   = "WIFI_REASON_ASSOC_EXPIRE",
    [5]   = "WIFI_REASON_ASSOC_TOOMANY",
    [6]   = "WIFI_REASON_NOT_AUTHED",
    [7]   = "WIFI_REASON_NOT_ASSOCED",
    [8]   = "WIFI_REASON_ASSOC_LEAVE",
    [9]   = "WIFI_REASON_ASSOC_NOT_AUTHED",
    [10]  = "WIFI_REASON_DISASSOC_PWRCAP_BAD",
    [11]  = "WIFI_REASON_DISASSOC_SUPCHAN_BAD",
    [12]  = "WIFI_REASON_BSS_TRANSITION_DISASSOC",
    [13]  = "WIFI_REASON_IE_INVALID",
    [14]  = "WIFI_REASON_MIC_FAILURE",
    [15]  = "WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT",
    [16]  = "WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT",
    [17]  = "WIFI_REASON_IE_IN_4WAY_DIFFERS",
    [18]  = "WIFI_REASON_GROUP_CIPHER_INVALID",
    [19]  = "WIFI_REASON_PAIRWISE_CIPHER_INVALID",
    [20]  = "WIFI_REASON_AKMP_INVALID",
    [21]  = "WIFI_REASON_UNSUPP_RSN_IE_VERSION",
    [22]  = "WIFI_REASON_INVALID_RSN_IE_CAP",
    [23]  = "WIFI_REASON_802_1X_AUTH_FAILED",
    [24]  = "WIFI_REASON_CIPHER_SUITE_REJECTED",
    [25]  = "WIFI_REASON_TDLS_PEER_UNREACHABLE",
    [26]  = "WIFI_REASON_TDLS_UNSPECIFIED",
    [27]  = "WIFI_REASON_SSP_REQUESTED_DISASSOC",
    [28]  = "WIFI_REASON_NO_SSP_ROAMING_AGREEMENT",
    [29]  = "WIFI_REASON_BAD_CIPHER_OR_AKM",
    [30]  = "WIFI_REASON_NOT_AUTHORIZED_THIS_LOCATION",
    [31]  = "WIFI_REASON_SERVICE_CHANGE_PERCLUDES_TS",
    [32]  = "WIFI_REASON_UNSPECIFIED_QOS",
    [33]  = "WIFI_REASON_NOT_ENOUGH_BANDWIDTH",
    [34]  = "WIFI_REASON_MISSING_ACKS",
    [35]  = "WIFI_REASON_EXCEEDED_TXOP",
    [36]  = "WIFI_REASON_STA_LEAVING",
    [37]  = "WIFI_REASON_END_BA",
    [38]  = "WIFI_REASON_UNKNOWN_BA",
    [39]  = "WIFI_REASON_TIMEOUT",
    [46]  = "WIFI_REASON_PEER_INITIATED",
    [47]  = "WIFI_REASON_AP_INITIATED",
    [48]  = "WIFI_REASON_INVALID_FT_ACTION_FRAME_COUNT",
    [49]  = "WIFI_REASON_INVALID_PMKID",
    [50]  = "WIFI_REASON_INVALID_MDE",
    [51]  = "WIFI_REASON_INVALID_FTE",
    [67]  = "WIFI_REASON_TRANSMISSION_LINK_ESTABLISH_FAILED",
    [68]  = "WIFI_REASON_ALTERATIVE_CHANNEL_OCCUPIED",
    [200] = "WIFI_REASON_BEACON_TIMEOUT",
    [201] = "WIFI_REASON_NO_AP_FOUND",
    [202] = "WIFI_REASON_AUTH_FAIL",
    [203] = "WIFI_REASON_ASSOC_FAIL",
    [204] = "WIFI_REASON_HANDSHAKE_TIMEOUT",
    [205] = "WIFI_REASON_CONNECTION_FAIL",
    [206] = "WIFI_REASON_AP_TSF_RESET",
    [207] = "WIFI_REASON_ROAMING",
    [208] = "WIFI_REASON_ASSOC_COMEBACK_TIME_TOO_LONG",
    [209] = "WIFI_REASON_SA_QUERY_TIMEOUT",
    [210] = "WIFI_REASON_NO_AP_FOUND_W_COMPATIBLE_SECURITY",
    [211] = "WIFI_REASON_NO_AP_FOUND_IN_AUTHMODE_THRESHOLD",
    [212] = "WIFI_REASON_NO_AP_FOUND_IN_RSSI_THRESHOLD",
};

// TODO(Caleb): JCFW OS
static EventGroupHandle_t           s_event_group        = NULL;
static esp_netif_t                 *s_sta_netif          = NULL;
static esp_event_handler_instance_t s_wifi_event_handler = NULL;
static esp_event_handler_instance_t s_ip_event_handler   = NULL;
static wifi_ap_record_t             s_scan_results[JCFW_WIFI_STA_SCAN_SIZE_MAX];

// -------------------------------------------------------------------------------------------------

typedef enum
{
    _JCFW_WIFI_STATUS_INITIALIZED   = JCFW_BIT(0),
    _JCFW_WIFI_STATUS_CONNECTED     = JCFW_BIT(1),
    _JCFW_WIFI_STATUS_DISCONNECTING = JCFW_BIT(2),
    _JCFW_WIFI_STATUS_DISCONNECTED  = JCFW_BIT(3),
    _JCFW_WIFI_STATUS_SCAN_IN_PROG  = JCFW_BIT(4),
    _JCFW_WIFI_STATUS_FAILURE       = JCFW_BIT(23),
} _jcfw_wifi_status_e;

// -------------------------------------------------------------------------------------------------

static void _jcfw_wifi_sta_event_handler(
    void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

static const char *_jcfw_wifi_get_authmode_string(wifi_auth_mode_t authmode);

static const char *_jcfw_wifi_get_reason_string(wifi_err_reason_t reason);

static void
_jcfw_wifi_convert_scan_result(jcfw_wifi_sta_scan_result_t *dest, wifi_ap_record_t *src);

// -------------------------------------------------------------------------------------------------

jcfw_result_e jcfw_wifi_init(void)
{
    JCFW_RETURN_IF_TRUE(jcfw_wifi_is_initialized(), JCFW_RESULT_OK);

    // NOTE(Caleb): The order of the following function calls is important and must be preserved.

    esp_err_t err;

    // TODO(Caleb): JCFW OS
    s_event_group = xEventGroupCreate();
    JCFW_ERROR_IF_FALSE(s_event_group, JCFW_RESULT_ERROR, "Unable to create an event group");

    err = esp_netif_init();
    JCFW_ERROR_IF_FALSE(
        err == ESP_OK,
        JCFW_RESULT_ERROR,
        "Unable to initialize the ESP32 TCP/IP stack (esp error %s)",
        esp_err_to_name(err));

    err = esp_event_loop_create_default();
    JCFW_ERROR_IF_FALSE(
        err == ESP_OK,
        JCFW_RESULT_ERROR,
        "Unable to initialize the ESP32 event loop (esp error %s)",
        esp_err_to_name(err));

    // NOTE(Caleb): May abort the program if a failure occurs.
    s_sta_netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    err                         = esp_wifi_init(&wifi_cfg);
    JCFW_ERROR_IF_FALSE(
        err == ESP_OK,
        JCFW_RESULT_ERROR,
        "Unable to initialize the ESP32 WIFI driver (esp error %s)",
        esp_err_to_name(err));

    err = esp_wifi_set_storage(WIFI_STORAGE_RAM);
    JCFW_ERROR_IF_FALSE(
        err == ESP_OK,
        JCFW_RESULT_ERROR,
        "Unable to set the WIFI storage location (esp error %s)",
        esp_err_to_name(err));

    err = esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, _jcfw_wifi_sta_event_handler, NULL, &s_wifi_event_handler);
    JCFW_ERROR_IF_FALSE(
        err == ESP_OK,
        JCFW_RESULT_ERROR,
        "Unable to register the WIFI event handler (esp error %s)",
        esp_err_to_name(err));

    err = esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, _jcfw_wifi_sta_event_handler, NULL, &s_ip_event_handler);
    JCFW_ERROR_IF_FALSE(
        err == ESP_OK,
        JCFW_RESULT_ERROR,
        "Unable to register the IP event handler (esp error %s)",
        esp_err_to_name(err));

    err = esp_wifi_set_mode(WIFI_MODE_APSTA);
    JCFW_ERROR_IF_FALSE(
        err == ESP_OK,
        JCFW_RESULT_ERROR,
        "Unable to set the WIFI mode (esp error %s)",
        esp_err_to_name(err));

    err = esp_wifi_start();
    JCFW_ERROR_IF_FALSE(
        err == ESP_OK,
        JCFW_RESULT_ERROR,
        "Unable to start the ESP32 WIFI driver (esp error %s)",
        esp_err_to_name(err));

    // TODO(Caleb): Wait for initialized bit to be set here.
    // TODO(Caleb): JCFW OS
    EventBits_t bits = xEventGroupWaitBits(
        s_event_group, _JCFW_WIFI_STATUS_INITIALIZED, pdFALSE, pdFALSE, portMAX_DELAY);

    JCFW_ERROR_IF_FALSE(
        bits & _JCFW_WIFI_STATUS_INITIALIZED,
        JCFW_RESULT_ERROR,
        "Unable to complete WIFI initialization (timeout)");

    return JCFW_RESULT_OK;
}

jcfw_result_e jcfw_wifi_deinit(void)
{
    JCFW_RETURN_IF_FALSE(jcfw_wifi_is_initialized(), JCFW_RESULT_OK);

    jcfw_result_e jcfw_err;
    esp_err_t     esp_err;

    if (jcfw_wifi_sta_is_connected())
    {
        jcfw_err = jcfw_wifi_sta_disconnect();

        // NOTE(Caleb): jcfw_wifi_sta_disconnect() already outputs an appropriate error message
        JCFW_RETURN_IF_FALSE(jcfw_err == JCFW_RESULT_OK, JCFW_RESULT_ERROR);
    }

    esp_err = esp_wifi_stop();
    JCFW_ERROR_IF_FALSE(
        esp_err == ESP_OK,
        JCFW_RESULT_ERROR,
        "Unable to stop the ESP32 WIFI driver (esp error %s)",
        esp_err_to_name(esp_err));

    esp_err = esp_wifi_set_mode(WIFI_MODE_NULL);
    JCFW_ERROR_IF_FALSE(
        esp_err == ESP_OK,
        JCFW_RESULT_ERROR,
        "Unable to clear the WIFI mode (esp error %s)",
        esp_err_to_name(esp_err));

    esp_err =
        esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, s_ip_event_handler);
    JCFW_ERROR_IF_FALSE(
        esp_err == ESP_OK,
        JCFW_RESULT_ERROR,
        "Unable to unregister the IP event handler (esp error %s)",
        esp_err_to_name(esp_err));

    esp_err =
        esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, s_wifi_event_handler);
    JCFW_ERROR_IF_FALSE(
        esp_err == ESP_OK,
        JCFW_RESULT_ERROR,
        "Unable to unregister the WIFI event handler (esp error %s)",
        esp_err_to_name(esp_err));

    esp_err = esp_wifi_deinit();
    JCFW_ERROR_IF_FALSE(
        esp_err == ESP_OK,
        JCFW_RESULT_ERROR,
        "Unable to deinitialize the ESP32 WIFI driver (esp error %s)",
        esp_err_to_name(esp_err));

    esp_netif_destroy_default_wifi(s_sta_netif);

    esp_err = esp_event_loop_delete_default();
    JCFW_ERROR_IF_FALSE(
        esp_err == ESP_OK,
        JCFW_RESULT_ERROR,
        "Unable to deinitialize the ESP32 event loop (esp error %s)",
        esp_err_to_name(esp_err));

    // NOTE(Caleb): Currently not supported by Espressif
    // esp_err = esp_netif_deinit();
    // JCFW_ERROR_IF_FALSE(
    //     esp_err == ESP_OK,
    //     JCFW_RESULT_ERROR,
    //     "Unable to deinitialize the ESP32 TCP/IP stack (esp error %s)",
    //     esp_err_to_name(esp_err));

    vEventGroupDelete(s_event_group);
    s_event_group = NULL;

    return JCFW_RESULT_OK;
}

jcfw_result_e jcfw_wifi_sta_connect(const char *ssid, const char *password)
{
    JCFW_ERROR_IF_FALSE(
        jcfw_wifi_is_initialized(), JCFW_RESULT_NOT_INITIALIZED, "WIFI is not initialized");
    JCFW_ERROR_IF_FALSE(ssid, JCFW_RESULT_INVALID_ARGS, "No SSID provided");

    // TODO(Caleb): Disconnect if already connected
    if (jcfw_wifi_sta_is_connected())
    {
        jcfw_result_e jcfw_err = jcfw_wifi_sta_disconnect();

        // NOTE(Caleb): jcfw_wifi_sta_disconnect() already outputs an appropriate error message
        JCFW_RETURN_IF_FALSE(jcfw_err == JCFW_RESULT_OK, JCFW_RESULT_ERROR);
    }

    esp_err_t         err;
    wifi_sta_config_t sta_cfg = {0};

    memcpy(sta_cfg.ssid, ssid, strnlen(ssid, sizeof(sta_cfg.ssid)));

    if (password)
    {
        memcpy(sta_cfg.password, password, strnlen(password, sizeof(sta_cfg.password)));
    }

    sta_cfg.threshold.authmode = (password) ? WIFI_AUTH_WPA_PSK : WIFI_AUTH_OPEN;

    wifi_config_t wifi_cfg = {.sta = sta_cfg};
    err                    = esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg);
    JCFW_ERROR_IF_FALSE(
        err == ESP_OK, JCFW_RESULT_ERROR, "Unable to configure the STA with new settings");

    err = esp_wifi_connect();
    JCFW_ERROR_IF_FALSE(
        err == ESP_OK,
        JCFW_RESULT_ERROR,
        "Unable to start the WIFI connect procedure (esp error %s)",
        esp_err_to_name(err));

    // TODO(Caleb): JCFW OS
    EventBits_t bits = xEventGroupWaitBits(
        s_event_group,
        _JCFW_WIFI_STATUS_CONNECTED | _JCFW_WIFI_STATUS_FAILURE,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY);

    JCFW_ERROR_IF_TRUE(
        bits & _JCFW_WIFI_STATUS_FAILURE,
        JCFW_RESULT_ERROR,
        "Unable to complete the WIFI connect procedure (max retries)");

    JCFW_ERROR_IF_FALSE(
        bits & _JCFW_WIFI_STATUS_CONNECTED,
        JCFW_RESULT_ERROR,
        "Unable to complete the WIFI connect procedure (timeout)");

    return JCFW_RESULT_OK;
}

jcfw_result_e jcfw_wifi_sta_disconnect(void)
{
    JCFW_ERROR_IF_FALSE(
        jcfw_wifi_is_initialized(), JCFW_RESULT_NOT_INITIALIZED, "WIFI is not initialized");

    xEventGroupSetBits(s_event_group, _JCFW_WIFI_STATUS_DISCONNECTING);

    esp_err_t err = esp_wifi_disconnect();
    JCFW_ERROR_IF_FALSE(
        err == ESP_OK,
        JCFW_RESULT_ERROR,
        "Unable to start the WIFI disconnect procedure (esp error %s)",
        esp_err_to_name(err));

    // TODO(Caleb): JCFW OS
    EventBits_t bits = xEventGroupWaitBits(
        s_event_group, _JCFW_WIFI_STATUS_DISCONNECTED, pdFALSE, pdFALSE, portMAX_DELAY);

    JCFW_ERROR_IF_FALSE(
        bits & _JCFW_WIFI_STATUS_DISCONNECTED,
        JCFW_RESULT_ERROR,
        "Unable to complete the WIFI disconnect procedure (timeout)");

    return JCFW_RESULT_OK;
}

jcfw_result_e jcfw_wifi_sta_scan(jcfw_wifi_sta_scan_result_t *o_aps, size_t *io_num_aps)
{
    JCFW_ERROR_IF_FALSE(
        jcfw_wifi_is_initialized(), JCFW_RESULT_NOT_INITIALIZED, "WIFI is not initialized");
    JCFW_ERROR_IF_FALSE(
        o_aps || io_num_aps || *io_num_aps > 0,
        JCFW_RESULT_INVALID_ARGS,
        "No memory provided for scan results");
    JCFW_ERROR_IF_FALSE(
        *io_num_aps <= JCFW_WIFI_STA_SCAN_SIZE_MAX,
        JCFW_RESULT_INVALID_ARGS,
        "Invalid scan result request size (requested %zu results, max request size %zu results)",
        *io_num_aps,
        JCFW_WIFI_STA_SCAN_SIZE_MAX);
    JCFW_ERROR_IF_TRUE(
        jcfw_wifi_sta_is_scanning(), JCFW_RESULT_IN_PROGRESS, "A scan is already in progress");

    xEventGroupSetBits(s_event_group, _JCFW_WIFI_STATUS_SCAN_IN_PROG);

    esp_err_t err;

    uint16_t num_aps = *io_num_aps;
    memset(s_scan_results, 0, sizeof(s_scan_results));

    err = esp_wifi_scan_start(NULL, true);
    JCFW_ERROR_IF_FALSE(
        err == ESP_OK, JCFW_RESULT_ERROR, "Unable to start the WIFI scan procedure");

    // NOTE(Caleb): The following will be refactored into a helper function when nonblocking
    // scanning is implemented

    err = esp_wifi_scan_get_ap_records(&num_aps, s_scan_results);
    JCFW_ERROR_IF_FALSE(err == ESP_OK, JCFW_RESULT_ERROR, "Unable to retrieve scan results");

    for (uint16_t i = 0; i < num_aps; i++)
    {
        _jcfw_wifi_convert_scan_result(&o_aps[i], &s_scan_results[i]);
    }

    *io_num_aps = num_aps;

    xEventGroupClearBits(s_event_group, _JCFW_WIFI_STATUS_SCAN_IN_PROG);

    return JCFW_RESULT_OK;
}

bool jcfw_wifi_is_initialized(void)
{
    JCFW_RETURN_IF_FALSE(s_event_group, false);

    // TODO(Caleb): JCFW OS
    EventBits_t bits = xEventGroupGetBits(s_event_group);
    return bits & _JCFW_WIFI_STATUS_INITIALIZED;
}

jcfw_result_e jcfw_wifi_sta_is_connected(void)
{
    JCFW_ERROR_IF_FALSE(jcfw_wifi_is_initialized(), false, "WIFI is not initialized");

    // TODO(Caleb): JCFW OS
    EventBits_t bits = xEventGroupGetBits(s_event_group);
    return bits & _JCFW_WIFI_STATUS_CONNECTED;
}

jcfw_result_e jcfw_wifi_sta_is_scanning(void)
{
    JCFW_ERROR_IF_FALSE(jcfw_wifi_is_initialized(), false, "WIFI is not initialized");

    // TODO(Caleb): JCFW OS
    EventBits_t bits = xEventGroupGetBits(s_event_group);
    return bits & _JCFW_WIFI_STATUS_SCAN_IN_PROG;
}

static void _jcfw_wifi_sta_event_handler(
    void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    JCFW_ERROR_IF_FALSE(
        event_base == WIFI_EVENT || event_base == IP_EVENT,
        /* ... */,
        "jcfw_wifi_sta event handler received unexpexted event base: %s",
        event_base);

    EventBits_t bits = xEventGroupGetBits(s_event_group);

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        JCFW_TRACELN_DEBUG(STA_TRACE_TAG, "WIFI STA driver has been initialized");

        // TODO(Caleb): JCFW OS
        xEventGroupSetBits(s_event_group, _JCFW_WIFI_STATUS_INITIALIZED);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        wifi_event_sta_connected_t *sta_conn_evt = event_data;

        JCFW_TRACELN_INFO(
            STA_TRACE_TAG,
            "STA connected to AP with SSID \"%.*s\". Waiting for IP address from DHCP...",
            sta_conn_evt->ssid_len,
            sta_conn_evt->ssid);

        JCFW_TRACELN_DEBUG(STA_TRACE_TAG, "Connection Info:");
        JCFW_TRACELN_DEBUG(
            STA_TRACE_TAG, "  SSID: %.*s", sta_conn_evt->ssid_len, sta_conn_evt->ssid);
        JCFW_TRACELN_DEBUG(
            STA_TRACE_TAG,
            "  MAC Address (BSSID): %02X:%02X:%02X:%02X:%02X:%02X",
            sta_conn_evt->bssid[0],
            sta_conn_evt->bssid[1],
            sta_conn_evt->bssid[2],
            sta_conn_evt->bssid[3],
            sta_conn_evt->bssid[4],
            sta_conn_evt->bssid[5]);
        JCFW_TRACELN_DEBUG(STA_TRACE_TAG, "  Channel: %u", sta_conn_evt->channel);
        JCFW_TRACELN_DEBUG(
            STA_TRACE_TAG,
            "  Auth. Mode: %u (%s)",
            sta_conn_evt->authmode,
            _jcfw_wifi_get_authmode_string(sta_conn_evt->authmode));
        JCFW_TRACELN_DEBUG(STA_TRACE_TAG, "  Assoc. ID: %hu", sta_conn_evt->aid);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        wifi_event_sta_disconnected_t *sta_disconn_evt = event_data;

        JCFW_TRACELN_INFO(
            STA_TRACE_TAG,
            "STA disconnected from AP with SSID \"%.*s\" (reason %u - %s)",
            sta_disconn_evt->ssid_len,
            sta_disconn_evt->ssid,
            sta_disconn_evt->reason,
            _jcfw_wifi_get_reason_string(sta_disconn_evt->reason));

        // TODO(Caleb): Check to see if the disconnect is intentional
        // TODO(Caleb): JCFW OS
        if (bits & _JCFW_WIFI_STATUS_CONNECTED)
        {
            xEventGroupClearBits(s_event_group, _JCFW_WIFI_STATUS_CONNECTED);
        }

        if (bits & _JCFW_WIFI_STATUS_DISCONNECTING)
        {
            xEventGroupClearBits(s_event_group, _JCFW_WIFI_STATUS_DISCONNECTING);
            xEventGroupSetBits(s_event_group, _JCFW_WIFI_STATUS_DISCONNECTED);
        }
        else
        {
            xEventGroupSetBits(s_event_group, _JCFW_WIFI_STATUS_FAILURE);
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *got_ip_evt = event_data;

        if (!got_ip_evt->ip_changed)
        {
            JCFW_TRACELN_INFO(
                STA_TRACE_TAG, "STA got an IP address: " IPSTR, IP2STR(&got_ip_evt->ip_info.ip));
        }
        else
        {
            JCFW_TRACELN_INFO(
                STA_TRACE_TAG, "STA got a new IP address: " IPSTR, IP2STR(&got_ip_evt->ip_info.ip));
        }

        JCFW_TRACELN_DEBUG(STA_TRACE_TAG, "IP Info:");
        JCFW_TRACELN_DEBUG(STA_TRACE_TAG, "  IP Address: " IPSTR, IP2STR(&got_ip_evt->ip_info.ip));
        JCFW_TRACELN_DEBUG(
            STA_TRACE_TAG, "  Subnet Mask: " IPSTR, IP2STR(&got_ip_evt->ip_info.netmask));
        JCFW_TRACELN_DEBUG(
            STA_TRACE_TAG, "  Gateway Address: " IPSTR, IP2STR(&got_ip_evt->ip_info.gw));

        // TODO(Caleb): JCFW OS
        if (bits & _JCFW_WIFI_STATUS_DISCONNECTED)
        {
            xEventGroupClearBits(s_event_group, _JCFW_WIFI_STATUS_DISCONNECTED);
        }

        xEventGroupSetBits(s_event_group, _JCFW_WIFI_STATUS_CONNECTED);
    }
    else
    {
        const char *event_base_str = (event_base == WIFI_EVENT) ? "WIFI" : "IP";
        JCFW_TRACELN_DEBUG(
            STA_TRACE_TAG, "No handler for %s event ID %lu", event_base_str, event_id);
    }
}

static const char *_jcfw_wifi_get_authmode_string(wifi_auth_mode_t authmode)
{
    return (authmode < sizeof(S_AUTHMODE_STRINGS)) ? S_AUTHMODE_STRINGS[authmode] : "UNKNOWN";
}

static const char *_jcfw_wifi_get_reason_string(wifi_err_reason_t reason)
{
    // NOTE(Caleb): Could result in indexing undefined memory (in the middle of the array), but
    // since the reason is coming from the library, I consider the input to be trusted
    const char *reason_string = S_REASON_STRINGS[reason];
    return (reason_string) ? reason_string : "UNKNOWN";
}

static void _jcfw_wifi_convert_scan_result(jcfw_wifi_sta_scan_result_t *dest, wifi_ap_record_t *src)
{
    JCFW_RETURN_IF_FALSE(dest && src);

    memset(dest, 0, sizeof(*dest));

    memccpy(dest->ssid, src->ssid, '\0', JCFW_WIFI_SSID_LEN_MAX);
    memcpy(dest->bssid, src->bssid, 6);
    dest->rssi_dBm = src->rssi;
    dest->channel  = src->primary;
}
