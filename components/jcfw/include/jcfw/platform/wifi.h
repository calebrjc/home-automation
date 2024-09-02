#ifndef __JCFW_PLATFORM_WIFI_H__
#define __JCFW_PLATFORM_WIFI_H__

#include "jcfw/detail/common.h"
#include "jcfw/util/result.h"

/* Notes:
 * jcfw_wifi_sta_*
 * jcfw_wifi_ap_*
 *
 * Configuration:
 * - retry count
 */

#define JCFW_WIFI_SSID_LEN_MAX      32
#define JCFW_WIFI_BSSID_LEN         6
#define JCFW_WIFI_STA_SCAN_SIZE_MAX 32

// TODO(Caleb): Add auth mode to this struct
typedef struct
{
    uint8_t ssid[JCFW_WIFI_SSID_LEN_MAX + 1];
    uint8_t bssid[JCFW_WIFI_BSSID_LEN];
    int8_t  rssi_dBm;
    uint8_t channel;
} jcfw_wifi_sta_scan_result_t;

jcfw_result_e jcfw_wifi_init(void);

jcfw_result_e jcfw_wifi_deinit(void);

jcfw_result_e jcfw_wifi_sta_connect(const char *ssid, const char *password);

jcfw_result_e jcfw_wifi_sta_disconnect(void);

// TODO(Caleb): Implement nonblocking WIFI scanning
// typedef void (*jcfw_wifi_sta_scan_cb_t)(jcfw_wifi_sta_scan_result_t *aps, size_t num_aps);

jcfw_result_e jcfw_wifi_sta_scan(jcfw_wifi_sta_scan_result_t *o_aps, size_t *io_num_aps);

bool jcfw_wifi_is_initialized(void);

jcfw_result_e jcfw_wifi_sta_is_connected(void);

jcfw_result_e jcfw_wifi_sta_is_scanning(void);

#endif // __JCFW_PLATFORM_WIFI_H__
