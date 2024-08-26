#include "jcfw/platform/platform.h"

#include <stdio.h>

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"

#include "jcfw/driver/als/ltr303.h"
#include "jcfw/util/assert.h"

#include "platform.h"

// -------------------------------------------------------------------------------------------------

#define ALS_INT_GPIO_NUM GPIO_NUM_23

// -------------------------------------------------------------------------------------------------

bool                           g_is_als_data_ready = false;
jcfw_ltr303_t                  g_ltr303            = {0};
static i2c_master_dev_handle_t s_als_i2c_handle    = NULL;

QueueHandle_t g_cli_uart_event_queue;

// -------------------------------------------------------------------------------------------------

static void on_als_data_ready(void *arg);

// -------------------------------------------------------------------------------------------------

jcfw_result_e jcfw_platform_init(void)
{
    jcfw_result_e jcfw_err;
    // esp_err_t     esp_err;

    gpio_config_t       gpio_cfg    = {0};
    i2c_device_config_t i2c_dev_cfg = {0};
    // uart_config_t       uart_cfg    = {0};

    i2c_master_bus_config_t i2c_bus_cfg = {
        .clk_source                   = I2C_CLK_SRC_DEFAULT,
        .i2c_port                     = I2C_NUM_1,
        .scl_io_num                   = GPIO_NUM_22,
        .sda_io_num                   = GPIO_NUM_21,
        .glitch_ignore_cnt            = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t i2c_bus_handle = NULL;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &i2c_bus_handle));

    // ALS ---------------------------------------------------------------------

    memset(&gpio_cfg, 0, sizeof(gpio_cfg));
    gpio_cfg.pin_bit_mask = 1 << ALS_INT_GPIO_NUM;
    gpio_cfg.mode         = GPIO_MODE_INPUT;
    gpio_cfg.intr_type    = GPIO_INTR_NEGEDGE;
    ESP_ERROR_CHECK(gpio_config(&gpio_cfg));
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(ALS_INT_GPIO_NUM, on_als_data_ready, NULL));

    memset(&i2c_dev_cfg, 0, sizeof(i2c_dev_cfg));
    i2c_dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    i2c_dev_cfg.device_address  = 0x29;
    i2c_dev_cfg.scl_speed_hz    = 400000;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &i2c_dev_cfg, &s_als_i2c_handle));

    jcfw_platform_delay_ms(100); // NOTE(Caleb): See LTR303 docs
    jcfw_err = jcfw_ltr303_init(&g_ltr303, s_als_i2c_handle, -1);
    JCFW_ASSERT_RET(jcfw_err == JCFW_RESULT_OK, JCFW_RESULT_ERROR);

    uint16_t thresh_low  = 0x0000;
    uint16_t thresh_high = 0x0000;
    jcfw_err             = jcfw_ltr303_set_thresholds(&g_ltr303, &thresh_low, &thresh_high);
    JCFW_ASSERT(jcfw_err == JCFW_RESULT_OK, "Unable to set thresholds");

    jcfw_err = jcfw_ltr303_enable_interrupt(&g_ltr303, true);
    JCFW_ASSERT(jcfw_err == JCFW_RESULT_OK, "Unable to enable interrupts");

    // CLI UART ----------------------------------------------------------------

    // TODO(Caleb):
    // For some reason this configuration causes the first TX'ed byte to not show up on the serial
    // monitor. Not sure why.

    // memset(&uart_cfg, 0, sizeof(uart_cfg));
    // uart_cfg.baud_rate  = 115200;
    // uart_cfg.data_bits  = UART_DATA_8_BITS;
    // uart_cfg.parity     = UART_PARITY_DISABLE;
    // uart_cfg.stop_bits  = UART_STOP_BITS_1;
    // uart_cfg.flow_ctrl  = UART_HW_FLOWCTRL_DISABLE;
    // uart_cfg.source_clk = UART_SCLK_DEFAULT;

    // esp_err = uart_driver_install(UART_NUM_0, 256, 0, 16, &g_cli_uart_event_queue, 0);
    // ESP_ERROR_CHECK(esp_err);

    // esp_err = uart_param_config(UART_NUM_0, &uart_cfg);
    // ESP_ERROR_CHECK(esp_err);

    // esp_err =
    //     uart_set_pin(UART_NUM_0, GPIO_NUM_1, GPIO_NUM_3, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    // ESP_ERROR_CHECK(esp_err);

    return JCFW_RESULT_OK;
}

void jcfw_platform_on_assert(const char *file, int line, const char *format, ...)
{
    printf("ASSERTION FAILED at %s:%d - ", file, line);

    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    abort();
}

bool jcfw_platform_trace_validate(const char *tag)
{
    // ...

    return true;
}

void jcfw_platform_delay_ms(uint32_t delay_ms)
{
    vTaskDelay(100 / portTICK_PERIOD_MS);
}

jcfw_result_e jcfw_platform_i2c_mstr_mem_read(
    void          *arg,
    const uint8_t *mem_addr,
    size_t         mem_addr_size,
    uint8_t       *o_data,
    size_t         data_size,
    uint32_t       timeout_ms)
{
    i2c_master_dev_handle_t handle = arg;

    esp_err_t err =
        i2c_master_transmit_receive(handle, mem_addr, mem_addr_size, o_data, data_size, timeout_ms);
    if (err != ESP_OK)
    {
        return JCFW_RESULT_ERROR;
    }

    return JCFW_RESULT_OK;
}

jcfw_result_e jcfw_platform_i2c_mstr_mem_write(
    void          *arg,
    const uint8_t *mem_addr,
    size_t         mem_addr_size,
    const uint8_t *data,
    size_t         data_size,
    uint32_t       timeout_ms)
{
    i2c_master_dev_handle_t handle = arg;

    uint8_t i2c_data[mem_addr_size + data_size];
    memcpy(i2c_data, mem_addr, mem_addr_size);
    memcpy(i2c_data + mem_addr_size, data, data_size);

    esp_err_t err = i2c_master_transmit(handle, i2c_data, mem_addr_size + data_size, timeout_ms);
    if (err != ESP_OK)
    {
        return JCFW_RESULT_ERROR;
    }

    return JCFW_RESULT_OK;
}

// -------------------------------------------------------------------------------------------------

static void on_als_data_ready(void *arg)
{
    g_is_als_data_ready = true;
}
