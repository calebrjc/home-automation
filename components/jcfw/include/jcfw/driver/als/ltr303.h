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
/// @note Be sure to wait until at least 100ms after power on to initialize the device (see:
/// datasheet page 25/26)
/// @param dev A pointer to the structure to initialize.
/// @param i2c_arg Optional; The argument to be used by the I2C platform function for this device.
/// @param i2c_timeout_ms The timeout to be used for I2C operations.
/// @return JCFW_RESULT_OK if the operation is successful, or an error code otherwise.
jcfw_result_e jcfw_ltr303_init(jcfw_ltr303_t *dev, void *i2c_arg, uint32_t i2c_timeout_ms);

/// @brief Perform a soft reset on the LTR303.
/// @param dev The device to perform the soft reset on.
/// @return JCFW_RESULT_OK if the operation is successful, or an error code otherwise.
jcfw_result_e jcfw_ltr303_reset(jcfw_ltr303_t *dev);

/// @brief Set the LTR303's running mode.
/// @param dev The device to change the running mode of.
/// @param mode The running mode to change the device to.
/// @return JCFW_RESULT_OK if the operation is successful, or an error code otherwise.
jcfw_result_e jcfw_ltr303_set_mode(jcfw_ltr303_t *dev, jcfw_ltr303_mode_e mode);

/// @brief Get the current running mode of the LTR303.
/// @param dev The device to get the current running mode of.
/// @param o_mode Required; The mode that the device is running in.
/// @return JCFW_RESULT_OK if the operation is successful, or an error code otherwise.
jcfw_result_e jcfw_ltr303_get_mode(jcfw_ltr303_t *dev, jcfw_ltr303_mode_e *o_mode);

/// @brief Get the data ready status of the LTR303.
/// @param dev The device to check the data ready status of.
/// @param o_is_data_ready Required; The data ready status of the device.
/// @return JCFW_RESULT_OK if the operation is successful, or an error code otherwise.
jcfw_result_e jcfw_ltr303_is_data_ready(jcfw_ltr303_t *dev, bool *o_is_data_ready);

/// @brief Read the registers of the LTR303. So long as the I2C operation is successful, the
/// registers will always be read, regardless of whether or not the data will be returned.
/// @note To calculate visible light, subtract channel 1 from channel 0. Be sure to handle clamping.
/// @param dev The device to read.
/// @param o_channel0_lux Optional; The data in channel 0 (visible + IR).
/// @param o_channel1_lux Optional; The data in channel 1 (IR only).
/// @return JCFW_RESULT_OK if the operation is successful, or an error code otherwise.
jcfw_result_e
jcfw_ltr303_read(jcfw_ltr303_t *dev, uint16_t *o_channel0_lux, uint16_t *o_channel1_lux);

/// @brief Enable or disable the interrupt of the LTR303.
/// @note If enabling, the user should configure a GPIO interrupt to handle notifications from the
/// device.
/// @param dev The device to enable interrupts for.
/// @param enable Whether or not interrupts should be enabled.
/// @return JCFW_RESULT_OK if the operation is successful, or an error code otherwise.
jcfw_result_e jcfw_ltr303_enable_interrupt(jcfw_ltr303_t *dev, bool enable);

/// @brief Set the interrupt polarity of the LTR303.
/// @param dev The device to set the interrupt polarity of.
/// @param polarity The polarity that should be set.
/// @return JCFW_RESULT_OK if the operation is successful, or an error code otherwise.
jcfw_result_e
jcfw_ltr303_set_interrupt_polarity(jcfw_ltr303_t *dev, jcfw_ltr303_interrupt_polarity_e polarity);

/// @brief Set the gain of the LTR303.
/// @param dev The device to set the gain of.
/// @param gain The gain to set for the device.
/// @return JCFW_RESULT_OK if the operation is successful, or an error code otherwise.
jcfw_result_e jcfw_ltr303_set_gain(jcfw_ltr303_t *dev, jcfw_ltr303_gain_e gain);

/// @brief Set the integration time of the LTR303. Refer to the datasheet for proper practice.
/// @param dev The device to set the integration time for.
/// @param integration_time The integration time to set for the device.
/// @return JCFW_RESULT_OK if the operation is successful, or an error code otherwise.
jcfw_result_e jcfw_ltr303_set_integration_time(
    jcfw_ltr303_t *dev, jcfw_ltr303_integration_time_e integration_time);

/// @brief Set the measurement rate of the LTR303. Refer to the datasheet for proper practice.
/// @param dev The device to set the measurement rate for.
/// @param measurement_rate The measurement rate to set for the device.
/// @return JCFW_RESULT_OK if the operation is successful, or an error code otherwise.
jcfw_result_e jcfw_ltr303_set_measurement_rate(
    jcfw_ltr303_t *dev, jcfw_ltr303_measurement_rate_e measurement_rate);

/// @brief Set the thresholds for the LTR303's interrupts.
/// @param dev The device to set the thresholds of.
/// @param threshold_low Optional; The low threshold to set. Defaults to 0x0000.
/// @param threshold_high Optional; The high threshold to set. Default to 0xFFFF.
/// @return JCFW_RESULT_OK if the operation is successful, or an error code otherwise.
jcfw_result_e jcfw_ltr303_set_thresholds(
    jcfw_ltr303_t *dev, const uint16_t *threshold_low, const uint16_t *threshold_high);

/// @brief Set the persistance for the LTR303's interrupts. After setting, interrupts will trigger
/// after n + 1 reads outside the thresholds.
/// @param dev The device to set the persistance of.
/// @param persistance The persistance to set. Range 0x00 - 0xFF. Defaults to 0x00.
/// @return JCFW_RESULT_OK if the operation is successful, or an error code otherwise.
jcfw_result_e jcfw_ltr303_set_persistance(jcfw_ltr303_t *dev, size_t persistance);

#endif // __JCFW_DRIVER_ALS_LTR303_H__
