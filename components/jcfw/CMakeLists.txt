set(JCFW_SRCS
    src/cli.c
    src/trace.c
    src/driver/als/ltr303.c
    src/platform/esp32/wifi.c)

idf_component_register(
    SRCS
    ${JCFW_SRCS}
    INCLUDE_DIRS
    "include"
    PRIV_REQUIRES
    esp_wifi)
