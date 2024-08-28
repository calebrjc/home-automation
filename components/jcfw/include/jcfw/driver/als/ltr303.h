#ifndef __JCFW_DRIVER_ALS_LTR303_H__
#define __JCFW_DRIVER_ALS_LTR303_H__

#include "jcfw/detail/common.h"

#include "jcfw/util/bit.h"
#include "jcfw/util/result.h"

#define JCFW_LTR303_INTR_PERSIST_DEFAULT 0x00

typedef struct
{
    void    *i2c_arg;
    uint32_t i2c_timeout_ms;
} jcfw_ltr303_t;

typedef enum
{
    JCFW_LTR303_MODE_STANDBY = 0,
    JCFW_LTR303_MODE_ACTIVE  = JCFW_BIT(0),
} jcfw_ltr303_mode_e;

typedef enum
{
    JCFW_LTR303_INTR_POL_LOW  = 0,
    JCFW_LTR303_INTR_POL_HIGH = JCFW_BIT(2),
} jcfw_ltr303_interrupt_polarity_e;

typedef enum
{
    JCFW_LTR303_GAIN_DEFAULT = 0x00,
    JCFW_LTR303_GAIN_1X      = 0x00,
    JCFW_LTR303_GAIN_2X      = 0x01,
    JCFW_LTR303_GAIN_4X      = 0x02,
    JCFW_LTR303_GAIN_8X      = 0x03,
    JCFW_LTR303_GAIN_48X     = 0x06,
    JCFW_LTR303_GAIN_96X     = 0x07,
} jcfw_ltr303_gain_e;

typedef enum
{
    JCFW_LTR303_INTEGRATION_TIME_DEFAULT = 0x00,
    JCFW_LTR303_INTEGRATION_TIME_100MS   = 0x00,
    JCFW_LTR303_INTEGRATION_TIME_50MS    = 0x01,
    JCFW_LTR303_INTEGRATION_TIME_200MS   = 0x02,
    JCFW_LTR303_INTEGRATION_TIME_400MS   = 0x03,
    JCFW_LTR303_INTEGRATION_TIME_150MS   = 0x04,
    JCFW_LTR303_INTEGRATION_TIME_250MS   = 0x05,
    JCFW_LTR303_INTEGRATION_TIME_300MS   = 0x06,
    JCFW_LTR303_INTEGRATION_TIME_3500MS  = 0x07,
} jcfw_ltr303_integration_time_e;

typedef enum
{
    JCFW_LTR303_MEAS_RATE_DEFAULT = 0x03,
    JCFW_LTR303_MEAS_RATE_50MS    = 0x00,
    JCFW_LTR303_MEAS_RATE_100MS   = 0x01,
    JCFW_LTR303_MEAS_RATE_200MS   = 0x02,
    JCFW_LTR303_MEAS_RATE_500MS   = 0x03,
    JCFW_LTR303_MEAS_RATE_1000MS  = 0x04,
    JCFW_LTR303_MEAS_RATE_2000MS  = 0x05,
} jcfw_ltr303_measurement_rate_e;

/// @brief Initialize the LTR303 driver.
/// @param dev A pointer to the structure to initialize.
/// @param i2c_arg Optional; The argument to be used by the I2C platform function for this device.
/// @param i2c_timeout_ms The timeout to be used for I2C operations.
/// @return JCFW_RESULT_OK if initialization is successful, or an error code otherwise.
jcfw_result_e jcfw_ltr303_init(jcfw_ltr303_t *dev, void *i2c_arg, uint32_t i2c_timeout_ms);

/// @brief Perform a soft reset on the LTR303.
/// @param dev The device to perform the soft reset on.
/// @return JCFW_RESULT_OK if the soft reset is successful, or an error code otherwise.
jcfw_result_e jcfw_ltr303_reset(jcfw_ltr303_t *dev);

jcfw_result_e jcfw_ltr303_set_mode(jcfw_ltr303_t *dev, jcfw_ltr303_mode_e mode);

jcfw_result_e jcfw_ltr303_get_mode(jcfw_ltr303_t *dev, jcfw_ltr303_mode_e *o_mode);

jcfw_result_e jcfw_ltr303_is_data_ready(jcfw_ltr303_t *dev, bool *o_is_data_ready);

jcfw_result_e jcfw_ltr303_read(jcfw_ltr303_t *dev, uint16_t *o_channel0, uint16_t *o_channel1);

jcfw_result_e jcfw_ltr303_enable_interrupt(jcfw_ltr303_t *dev, bool enable);

jcfw_result_e
jcfw_ltr303_set_interrupt_polarity(jcfw_ltr303_t *dev, jcfw_ltr303_interrupt_polarity_e polarity);

jcfw_result_e jcfw_ltr303_set_gain(jcfw_ltr303_t *dev, jcfw_ltr303_gain_e gain);

jcfw_result_e jcfw_ltr303_set_integration_time(
    jcfw_ltr303_t *dev, jcfw_ltr303_integration_time_e integration_time);

jcfw_result_e jcfw_ltr303_set_measurement_rate(
    jcfw_ltr303_t *dev, jcfw_ltr303_measurement_rate_e measurement_rate);

jcfw_result_e jcfw_ltr303_set_thresholds(
    jcfw_ltr303_t *dev, const uint16_t *threshold_low, const uint16_t *threshold_high);

jcfw_result_e jcfw_ltr303_set_persistance(jcfw_ltr303_t *dev, size_t persistance);

#endif // __JCFW_DRIVER_ALS_LTR303_H__
