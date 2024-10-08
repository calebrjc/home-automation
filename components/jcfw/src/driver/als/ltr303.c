#include "jcfw/driver/als/ltr303.h"

#include "jcfw/platform/platform.h"
#include "jcfw/util/assert.h"
#include "jcfw/util/math.h"

typedef enum
{
    JCFW_LTR303_REG_ALS_CONTR             = 0x80,
    JCFW_LTR303_REG_ALS_MEAS_RATE         = 0x85,
    JCFW_LTR303_REG_PART_ID               = 0x86,
    JCFW_LTR303_REG_MANUFAC_ID            = 0x87,
    JCFW_LTR303_REG_ALS_DATA_CH1_0        = 0x88,
    JCFW_LTR303_REG_ALS_DATA_CH1_1        = 0x89,
    JCFW_LTR303_REG_ALS_DATA_CH0_0        = 0x8A,
    JCFW_LTR303_REG_ALS_DATA_CH0_1        = 0x8B,
    JCFW_LTR303_REG_ALS_STATUS            = 0x8C,
    JCFW_LTR303_REG_ALS_INTERRUPT         = 0x8F,
    JCFW_LTR303_REG_ALS_THRES_UP_0        = 0x97,
    JCFW_LTR303_REG_ALS_THRES_UP_1        = 0x98,
    JCFW_LTR303_REG_ALS_THRES_LOW_0       = 0x99,
    JCFW_LTR303_REG_ALS_THRES_LOW_1       = 0x9A,
    JCFW_LTR303_REG_ALS_INTERRUPT_PERSIST = 0x9E,
} jcfw_ltr303_register_e;

static uint8_t S_GAIN_FACTORS[] = {
    [JCFW_LTR303_GAIN_1X]  = 1,
    [JCFW_LTR303_GAIN_2X]  = 2,
    [JCFW_LTR303_GAIN_4X]  = 4,
    [JCFW_LTR303_GAIN_8X]  = 8,
    [JCFW_LTR303_GAIN_48X] = 48,
    [JCFW_LTR303_GAIN_96X] = 96,
};

jcfw_result_e jcfw_ltr303_init(jcfw_ltr303_t *dev, void *i2c_arg, uint32_t i2c_timeout_ms)
{
    JCFW_ERROR_IF_FALSE(dev, JCFW_RESULT_INVALID_ARGS, "No device provided");

    dev->i2c_arg        = i2c_arg;
    dev->i2c_timeout_ms = i2c_timeout_ms;

    jcfw_result_e err;

    uint8_t reg     = JCFW_LTR303_REG_PART_ID;
    uint8_t part_id = 0x00;
    err = jcfw_platform_i2c_mstr_mem_read(dev->i2c_arg, &reg, 1, &part_id, 1, dev->i2c_timeout_ms);
    JCFW_ERROR_IF_FALSE(err == JCFW_RESULT_OK, err, "I2C read operation failed (jcfw rc %u)", err);
    JCFW_ERROR_IF_FALSE(
        part_id == 0xA0,
        JCFW_RESULT_ERROR, // NOTE(Caleb): Should this be a different error?
        "LTR303 - Invalid part ID %02x (expected 0xA0)",
        part_id);

    reg            = JCFW_LTR303_REG_MANUFAC_ID;
    uint8_t man_id = 0x00;
    err = jcfw_platform_i2c_mstr_mem_read(dev->i2c_arg, &reg, 1, &man_id, 1, dev->i2c_timeout_ms);
    JCFW_ERROR_IF_FALSE(err == JCFW_RESULT_OK, err, "I2C read operation failed (jcfw rc %u)", err);
    JCFW_ERROR_IF_FALSE(
        man_id == 0x05,
        JCFW_RESULT_ERROR, // NOTE(Caleb): Should this be a different error?
        "LTR303 - Invalid manufacturer ID %02x (expected 0x05)",
        man_id);

    return jcfw_ltr303_reset(dev);
}

jcfw_result_e jcfw_ltr303_reset(jcfw_ltr303_t *dev)
{
    JCFW_ERROR_IF_FALSE(dev, JCFW_RESULT_INVALID_ARGS, "No device provided");

    jcfw_result_e err;

    uint8_t reg = JCFW_LTR303_REG_ALS_CONTR;
    uint8_t cmd = JCFW_BIT(1);
    err = jcfw_platform_i2c_mstr_mem_write(dev->i2c_arg, &reg, 1, &cmd, 1, dev->i2c_timeout_ms);
    JCFW_ERROR_IF_FALSE(err == JCFW_RESULT_OK, err, "I2C write operation failed (jcfw rc %u)", err);

    jcfw_platform_delay_ms(10); // See datasheet page 25/26

    return JCFW_RESULT_OK;
}

jcfw_result_e jcfw_ltr303_set_mode(jcfw_ltr303_t *dev, jcfw_ltr303_mode_e mode)
{
    JCFW_ERROR_IF_FALSE(dev, JCFW_RESULT_INVALID_ARGS, "No device provided");

    jcfw_result_e err;
    uint8_t       reg         = JCFW_LTR303_REG_ALS_CONTR;
    uint8_t       control_reg = 0x00;

    err = jcfw_platform_i2c_mstr_mem_read(
        dev->i2c_arg, &reg, 1, &control_reg, 1, dev->i2c_timeout_ms);
    JCFW_ERROR_IF_FALSE(err == JCFW_RESULT_OK, err, "I2C read operation failed (jcfw rc %u)", err);

    if (mode == JCFW_LTR303_MODE_ACTIVE)
    {
        JCFW_BITSET(control_reg, JCFW_BIT(0));
    }
    else
    {
        JCFW_BITCLEAR(control_reg, JCFW_BIT(0));
    }

    err = jcfw_platform_i2c_mstr_mem_write(
        dev->i2c_arg, &reg, 1, &control_reg, 1, dev->i2c_timeout_ms);
    JCFW_ERROR_IF_FALSE(err == JCFW_RESULT_OK, err, "I2C write operation failed (jcfw rc %u)", err);

    return JCFW_RESULT_OK;
}

jcfw_result_e jcfw_ltr303_get_mode(jcfw_ltr303_t *dev, jcfw_ltr303_mode_e *o_mode)
{
    JCFW_ERROR_IF_FALSE(dev, JCFW_RESULT_INVALID_ARGS, "No device provided");
    JCFW_ERROR_IF_FALSE(
        o_mode, JCFW_RESULT_INVALID_ARGS, "No memory provided for required return values");

    uint8_t reg         = JCFW_LTR303_REG_ALS_CONTR;
    uint8_t control_reg = 0x00;

    jcfw_result_e err = jcfw_platform_i2c_mstr_mem_read(
        dev->i2c_arg, &reg, 1, &control_reg, 1, dev->i2c_timeout_ms);
    JCFW_ERROR_IF_FALSE(err == JCFW_RESULT_OK, err, "I2C read operation failed (jcfw rc %u)", err);

    *o_mode = (control_reg & JCFW_BIT(0)) ? JCFW_LTR303_MODE_ACTIVE : JCFW_LTR303_MODE_STANDBY;
    return JCFW_RESULT_OK;
}

jcfw_result_e jcfw_ltr303_is_data_ready(jcfw_ltr303_t *dev, bool *o_is_data_ready)
{
    JCFW_ERROR_IF_FALSE(dev, JCFW_RESULT_INVALID_ARGS, "No device provided");
    JCFW_ERROR_IF_FALSE(
        o_is_data_ready, JCFW_RESULT_INVALID_ARGS, "No memory provided for required return values");

    *o_is_data_ready = false;

    uint8_t reg    = JCFW_LTR303_REG_ALS_STATUS;
    uint8_t status = 0x00;

    jcfw_result_e err =
        jcfw_platform_i2c_mstr_mem_read(dev->i2c_arg, &reg, 1, &status, 1, dev->i2c_timeout_ms);
    JCFW_ERROR_IF_FALSE(err == JCFW_RESULT_OK, err, "I2C read operation failed (jcfw rc %u)", err);

    if (status & JCFW_BIT(2) || status & JCFW_BIT(3)) // NEW/INT
    {
        *o_is_data_ready = true;
    }

    return JCFW_RESULT_OK;
}

jcfw_result_e jcfw_ltr303_read(
    jcfw_ltr303_t *dev, uint16_t *o_channel0_lux, uint16_t *o_channel1_lux, uint8_t *o_gain_factor)
{
    JCFW_ERROR_IF_FALSE(dev, JCFW_RESULT_INVALID_ARGS, "No device provided");

    // TODO(Caleb): May have to add a byteswap to make this function endianness-independent

    jcfw_result_e err;
    uint8_t       reg;

    if (o_gain_factor)
    {
        reg            = JCFW_LTR303_REG_ALS_STATUS;
        uint8_t status = 0x00;

        jcfw_result_e err =
            jcfw_platform_i2c_mstr_mem_read(dev->i2c_arg, &reg, 1, &status, 1, dev->i2c_timeout_ms);
        JCFW_ERROR_IF_FALSE(
            err == JCFW_RESULT_OK, err, "I2C read operation failed (jcfw rc %u)", err);

        jcfw_ltr303_gain_e gain = (status >> 4) & 0x07; // See datasheet page 21
        *o_gain_factor          = S_GAIN_FACTORS[gain];
    }

    reg             = JCFW_LTR303_REG_ALS_DATA_CH1_0;
    uint16_t data[] = {0x0000, 0x0000};

    err = jcfw_platform_i2c_mstr_mem_read(
        dev->i2c_arg, &reg, 1, (uint8_t *)data, 4, dev->i2c_timeout_ms);
    JCFW_ERROR_IF_FALSE(err == JCFW_RESULT_OK, err, "I2C read operation failed (jcfw rc %u)", err);

    if (o_channel0_lux)
    {
        *o_channel0_lux = data[1];
    }

    if (o_channel1_lux)
    {
        *o_channel1_lux = data[0];
    }

    return JCFW_RESULT_OK;
}

jcfw_result_e jcfw_ltr303_enable_interrupt(jcfw_ltr303_t *dev, bool enable)
{
    JCFW_ERROR_IF_FALSE(dev, JCFW_RESULT_INVALID_ARGS, "No device provided");

    jcfw_result_e err;
    uint8_t       reg           = JCFW_LTR303_REG_ALS_INTERRUPT;
    uint8_t       interrupt_cfg = 0x00;

    err = jcfw_platform_i2c_mstr_mem_read(
        dev->i2c_arg, &reg, 1, &interrupt_cfg, 1, dev->i2c_timeout_ms);
    JCFW_ERROR_IF_FALSE(err == JCFW_RESULT_OK, err, "I2C read operation failed (jcfw rc %u)", err);

    if (enable)
    {
        JCFW_BITSET(interrupt_cfg, JCFW_BIT(1));
    }
    else
    {
        JCFW_BITCLEAR(interrupt_cfg, JCFW_BIT(1));
    }

    err = jcfw_platform_i2c_mstr_mem_write(
        dev->i2c_arg, &reg, 1, &interrupt_cfg, 1, dev->i2c_timeout_ms);
    JCFW_ERROR_IF_FALSE(err == JCFW_RESULT_OK, err, "I2C write operation failed (jcfw rc %u)", err);

    return JCFW_RESULT_OK;
}

jcfw_result_e
jcfw_ltr303_set_interrupt_polarity(jcfw_ltr303_t *dev, jcfw_ltr303_interrupt_polarity_e polarity)
{
    JCFW_ERROR_IF_FALSE(dev, JCFW_RESULT_INVALID_ARGS, "No device provided");

    jcfw_result_e err;
    uint8_t       reg           = JCFW_LTR303_REG_ALS_INTERRUPT;
    uint8_t       interrupt_cfg = 0x00;

    err = jcfw_platform_i2c_mstr_mem_read(
        dev->i2c_arg, &reg, 1, &interrupt_cfg, 1, dev->i2c_timeout_ms);
    JCFW_ERROR_IF_FALSE(err == JCFW_RESULT_OK, err, "I2C read operation failed (jcfw rc %u)", err);

    if (polarity == JCFW_LTR303_INTR_POL_HIGH)
    {
        JCFW_BITSET(interrupt_cfg, JCFW_BIT(2));
    }
    else
    {
        JCFW_BITCLEAR(interrupt_cfg, JCFW_BIT(2));
    }

    err = jcfw_platform_i2c_mstr_mem_write(
        dev->i2c_arg, &reg, 1, &interrupt_cfg, 1, dev->i2c_timeout_ms);
    JCFW_ERROR_IF_FALSE(err == JCFW_RESULT_OK, err, "I2C write operation failed (jcfw rc %u)", err);

    return JCFW_RESULT_OK;
}

jcfw_result_e jcfw_ltr303_set_gain(jcfw_ltr303_t *dev, jcfw_ltr303_gain_e gain)
{
    JCFW_ERROR_IF_FALSE(dev, JCFW_RESULT_INVALID_ARGS, "No device provided");

    jcfw_result_e err;
    uint8_t       reg         = JCFW_LTR303_REG_ALS_CONTR;
    uint8_t       control_reg = 0x00;

    err = jcfw_platform_i2c_mstr_mem_read(
        dev->i2c_arg, &reg, 1, &control_reg, 1, dev->i2c_timeout_ms);
    JCFW_ERROR_IF_FALSE(err == JCFW_RESULT_OK, err, "I2C read operation failed (jcfw rc %u)", err);

    JCFW_BITCLEAR(control_reg, 0x1C /* 0b00011100 */);
    JCFW_BITSET(control_reg, (uint8_t)gain << 2);

    err = jcfw_platform_i2c_mstr_mem_write(
        dev->i2c_arg, &reg, 1, &control_reg, 1, dev->i2c_timeout_ms);
    JCFW_ERROR_IF_FALSE(err == JCFW_RESULT_OK, err, "I2C write operation failed (jcfw rc %u)", err);

    return JCFW_RESULT_OK;
}

jcfw_result_e jcfw_ltr303_set_integration_time(
    jcfw_ltr303_t *dev, jcfw_ltr303_integration_time_e integration_time)
{
    JCFW_ERROR_IF_FALSE(dev, JCFW_RESULT_INVALID_ARGS, "No device provided");

    jcfw_result_e err;
    uint8_t       reg           = JCFW_LTR303_REG_ALS_MEAS_RATE;
    uint8_t       meas_rate_reg = 0x00;

    err = jcfw_platform_i2c_mstr_mem_read(
        dev->i2c_arg, &reg, 1, &meas_rate_reg, 1, dev->i2c_timeout_ms);
    JCFW_ERROR_IF_FALSE(err == JCFW_RESULT_OK, err, "I2C read operation failed (jcfw rc %u)", err);

    JCFW_BITCLEAR(meas_rate_reg, 0x38 /* 0b00111000 */);
    JCFW_BITSET(meas_rate_reg, (uint8_t)integration_time << 3);

    err = jcfw_platform_i2c_mstr_mem_write(
        dev->i2c_arg, &reg, 1, &meas_rate_reg, 1, dev->i2c_timeout_ms);
    JCFW_ERROR_IF_FALSE(err == JCFW_RESULT_OK, err, "I2C write operation failed (jcfw rc %u)", err);

    return JCFW_RESULT_OK;
}

jcfw_result_e jcfw_ltr303_set_measurement_rate(
    jcfw_ltr303_t *dev, jcfw_ltr303_measurement_rate_e measurement_rate)
{
    JCFW_ERROR_IF_FALSE(dev, JCFW_RESULT_INVALID_ARGS, "No device provided");

    jcfw_result_e err;
    uint8_t       reg           = JCFW_LTR303_REG_ALS_MEAS_RATE;
    uint8_t       meas_rate_reg = 0x00;

    err = jcfw_platform_i2c_mstr_mem_read(
        dev->i2c_arg, &reg, 1, &meas_rate_reg, 1, dev->i2c_timeout_ms);
    JCFW_ERROR_IF_FALSE(err == JCFW_RESULT_OK, err, "I2C read operation failed (jcfw rc %u)", err);

    JCFW_BITCLEAR(meas_rate_reg, 0x07 /* 0b00000111 */);
    JCFW_BITSET(meas_rate_reg, (uint8_t)measurement_rate << 3);

    err = jcfw_platform_i2c_mstr_mem_write(
        dev->i2c_arg, &reg, 1, &meas_rate_reg, 1, dev->i2c_timeout_ms);
    JCFW_ERROR_IF_FALSE(err == JCFW_RESULT_OK, err, "I2C write operation failed (jcfw rc %u)", err);

    return JCFW_RESULT_OK;
}

jcfw_result_e jcfw_ltr303_set_thresholds(
    jcfw_ltr303_t *dev, const uint16_t *threshold_low, const uint16_t *threshold_high)
{
    JCFW_ERROR_IF_FALSE(dev, JCFW_RESULT_INVALID_ARGS, "No device provided");

    // TODO(Caleb): May have to add a byteswap to make this function endianness-independent

    jcfw_result_e err;
    uint8_t       reg;

    if (threshold_low)
    {
        reg = JCFW_LTR303_REG_ALS_THRES_LOW_0;
        err = jcfw_platform_i2c_mstr_mem_write(
            dev->i2c_arg, &reg, 1, (uint8_t *)threshold_low, 2, dev->i2c_timeout_ms);
        JCFW_ERROR_IF_FALSE(
            err == JCFW_RESULT_OK, err, "I2C write operation failed (jcfw rc %u)", err);
    }

    if (threshold_high)
    {
        reg = JCFW_LTR303_REG_ALS_THRES_UP_0;
        err = jcfw_platform_i2c_mstr_mem_write(
            dev->i2c_arg, &reg, 1, (uint8_t *)threshold_high, 2, dev->i2c_timeout_ms);
        JCFW_ERROR_IF_FALSE(
            err == JCFW_RESULT_OK, err, "I2C write operation failed (jcfw rc %u)", err);
    }

    return JCFW_RESULT_OK;
}

jcfw_result_e jcfw_ltr303_set_persistance(jcfw_ltr303_t *dev, size_t persistance)
{
    JCFW_ERROR_IF_FALSE(dev, JCFW_RESULT_INVALID_ARGS, "No device provided");

    uint8_t reg = JCFW_LTR303_REG_ALS_INTERRUPT_PERSIST;
    persistance = JCFW_CLAMP(persistance, 0, 15);

    jcfw_result_e err = jcfw_platform_i2c_mstr_mem_write(
        dev->i2c_arg, &reg, 1, (uint8_t *)&persistance, 1, dev->i2c_timeout_ms);
    JCFW_ERROR_IF_FALSE(err == JCFW_RESULT_OK, err, "I2C write operation failed (jcfw rc %u)", err);

    return JCFW_RESULT_OK;
}
