#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include "nau.h"
#define I2C_NODE DT_NODELABEL(nau)

#define pH_val_buff_size 10
#define infection_detect_buff_size 10

LOG_MODULE_REGISTER(nau_code, LOG_LEVEL_INF);

static const struct i2c_dt_spec nau = I2C_DT_SPEC_GET(I2C_NODE);

uint8_t pH_infection_check[infection_detect_buff_size];



bool nau_init()
{
    int ret = 0;
    uint8_t reg = NAU_DEV_CODE;
    uint8_t data;

    if (!device_is_ready(nau.bus))
    {
        // LOG_ERR("I2C bus %s is not ready!", nau.bus->name);
        return false;
    }

    k_msleep(1000);
    ret = i2c_write_read_dt(&nau, &reg, 1, &data, 1);
    if (ret != 0)
    {
        // LOG_ERR("Couldn't read the device address");
        return false;
    }

    // LOG_INF("Device ID: %02x", data);

    k_msleep(100);

    // Reseting the device registers
    uint8_t reg_write[2];
    reg_write[0] = NAU_PU_CTRL;
    reg_write[1] = 0x01;
    ret = i2c_write_dt(&nau, &reg_write, sizeof(reg_write));

    if (ret != 0)
    {
        // LOG_ERR("Failed to reset the device registers");
        return false;
    }

    k_msleep(100);

    // PowerOn sequence
    reg_write[0] = NAU_PU_CTRL;
    reg_write[1] = 0x96;
    ret = i2c_write_dt(&nau, &reg_write, sizeof(reg_write));
    if (ret != 0)
    {
        // LOG_ERR("Failed to initiate power-up sequence");
        return false;
    }

    k_msleep(100);
    ret = i2c_write_read_dt(&nau, &reg_write, 1, &data, 1);
    if (ret != 0)
    {
        // LOG_ERR("Couldn't read the device address");
        return false;
    }
    // LOG_INF("R0x00 powerup: %02x", data);

    // Setting LOW ESR for high precision at lower gain values
    reg_write[0] = NAU_PGA;
    reg_write[1] = 0x00 | BIT(6);
    ret = i2c_write_dt(&nau, &reg_write, sizeof(reg_write));
    if (ret != 0)
    {
        // LOG_ERR("Couldn't read the device address");
        return false;
    }

    // PGA_OUTPUT_CAP_ON
    reg_write[0] = NAU7802_POWER_CAP;
    reg_write[1] = 0x00 | BIT(7);
    ret = i2c_write_dt(&nau, &reg_write, sizeof(reg_write));
    if (ret != 0)
    {
        // LOG_ERR("Couldn't read the device address");
        return false;
    }

    return true;
}

void nau_ctrl_reg(uint8_t gain, uint8_t VLDO, uint8_t CR, uint8_t CALMOD)
{
    int ret = 0;
    uint8_t reg[2];
    uint8_t data;
    reg[0] = NAU_CTRL1; // Writes gain and VLDO values to reg
    reg[1] = 0x00;

    reg[1] = VLDO << 3 | gain;
    // Writing data to CTRL1
    ret = i2c_write_dt(&nau, &reg, sizeof(reg));
    if (ret != 0)
    {
        // LOG_ERR("Couldn't write to reg %02x", reg[0]);
        return;
    }

    k_msleep(300);
    // Reading data from CTRL1
    ret = i2c_write_read_dt(&nau, &reg, 1, &data, 1);
    if (ret != 0)
    {
        // LOG_ERR("Couldn't read the device address");
        return;
    }

    // LOG_INF("Data read from CTRL1: %02x", data);

    k_msleep(100);

    reg[0] = NAU_CTRL2;
    reg[1] = 0x00;

    reg[1] = CR << 4 | 1 << 3 | CALMOD;
    // Writing data to CTRL2
    ret = i2c_write_dt(&nau, &reg, sizeof(reg));
    if (ret != 0)
    {
        // LOG_ERR("Couldn't write to reg %02x", reg[0]);
        return;
    }

    k_msleep(300);
    // Reading data from CTRL2
    ret = i2c_write_read_dt(&nau, &reg, 1, &data, 1);
    if (ret != 0)
    {
        // LOG_ERR("Couldn't read the device address");
        return;
    }

    // LOG_INF("Data read from CTRL2: %02x", data);

    if ((data & 1 << 2) == 0)
    {
        if ((data & 1 << 3) == 8)
        {
            // LOG_INF("Calibration failed");
        }
        else
        {
            // LOG_INF("Calibration successful");
        }
    }
    else
    {
        // LOG_INF("Calibration in progress");
    }
    // LOG_INF("Control Register's settings complete.");
}

void nau_offset_calib()
{
    int ret = 0;
    uint8_t reg[2];
    uint8_t data;
    reg[0] = NAU_CTRL2; // Writes gain and VLDO values to reg
    reg[1] = 0x00 | BIT(1) | BIT(2) | BIT(5);

    // Writing data to CTRL2
    ret = i2c_write_dt(&nau, &reg, sizeof(reg));
    if (ret != 0)
    {
        // LOG_ERR("Couldn't write to reg %02x", reg[0]);
        printk("Couldn't write to reg %02x", reg[0]);
        return;
    }

    ret = i2c_write_read_dt(&nau, &reg, 1, &data, 1);
    if (ret != 0)
    {
        // LOG_ERR("Couldn't read the device address");
        return;
    }

    k_msleep(1000);

    if ((data & BIT(2)) == 0)
    {
        if (data && BIT(3))
        {
            LOG_INF("Calibration failed");
        }
        else
        {
            LOG_INF("Calibration successful");
        }
    }
    else
    {
        LOG_INF("Calibration in progress");
    }
}

bool data_ready()
{
    int ret = 0;
    uint8_t reg = NAU_PU_CTRL;
    uint8_t read_reg_pu;
    ret = i2c_write_read_dt(&nau, &reg, 1, &read_reg_pu, 1);
    if (ret != 0)
    {
        // LOG_ERR("Couldn't read the device address");
        return false;
    }
    if (read_reg_pu && BIT(5))
    {
        return true;
    }
    else
    {
        return false;
    }
}

uint32_t nau_Read_ADC()
{
    int ret = 0;
    uint32_t adc_data = 0;
    uint8_t reg = NAU_ADCO_B2;
    uint8_t data[3];
    uint8_t read_reg_pu;
    ret = i2c_write_read_dt(&nau, &reg, 1, &data, 3);
    if (ret != 0)
    {
        // LOG_ERR("Couldn't read the device address");
        return false;
    }
    adc_data = data[0];
    adc_data = adc_data << 8 | data[1];
    adc_data = adc_data << 8 | data[2];

    // LOG_INF("ADC_DATA: %06x", adc_data);

    // Reading R0x00[5] to check whether ADC data is ready
    reg = NAU_PU_CTRL;
    ret = i2c_write_read_dt(&nau, &reg, 1, &read_reg_pu, 1);
    if (ret != 0)
    {
        // LOG_ERR("Couldn't read the device address");
        return false;
    }
    // LOG_INF("R0x00 Data: %02x", read_reg_pu);
    uint32_t adc_data_dec;
    // LOG_INF("ADC data: %06x", adc_data);
    if (adc_data & 0x800000)
    {
        // Sign extend to 32-bit by setting upper 8 bits to 1
        adc_data_dec = (int32_t)(adc_data | 0xFF000000);
    }
    else
    {
        adc_data_dec = (int32_t)adc_data;
    }
    // printk("%06x , ", adc_data);
    // printk("adc data %d \r\n", adc_data_dec);
    return adc_data_dec;
}


void set_pH_buff(uint8_t pH_val)
{
    static uint8_t pH_val_ctr = 0;
    pH_val_ctr = pH_val_ctr < infection_detect_buff_size ? pH_val_ctr : 0;
    pH_infection_check[pH_val_ctr] = pH_val;
    pH_val_ctr++;
}

struct pH_and_infection raw_data_to_pH(void)
{
    uint8_t pH_val = 0;
    static uint8_t pH_prev_val = 0;
    pH_and_infection pH_inf;
    int raw_data = 0;
    uint32_t raw_data_buff[pH_val_buff_size];

    for (int i = 0; i < pH_val_buff_size; i++)
    {
        raw_data_buff[i] = nau_Read_ADC();
    }

    for (int i = 0; i < pH_val_buff_size; i++)
    {
        raw_data += raw_data_buff[i];
    }

    raw_data /= pH_val_buff_size;

    if ((raw_data >= pH_1_L))
    {
        pH_val = 1;
    }
    else if ((raw_data >= pH_2_L) && (raw_data <= pH_2_H))
    {
        pH_val = 2;
    }
    else if ((raw_data >= pH_3_L) && (raw_data <= pH_3_H))
    {
        pH_val = 3;
    }
    else if ((raw_data >= pH_4_L) && (raw_data <= pH_4_H))
    {
        pH_val = 4;
    }
    else if ((raw_data >= pH_5_L) && (raw_data <= pH_5_H))
    {
        pH_val = 5;
    }
    else if ((raw_data >= pH_6_L) && (raw_data <= pH_6_H))
    {
        pH_val = 6;
    }
    else if ((raw_data >= pH_7_L) && (raw_data <= pH_7_H))
    {
        pH_val = 7;
    }
    else if ((raw_data >= pH_8_L) && (raw_data <= pH_8_H))
    {
        pH_val = 8;
    }
    else if ((raw_data >= pH_9_L) && (raw_data <= pH_9_H))
    {
        pH_val = 9;
    }

    else if ((raw_data >= pH_10_L) && (raw_data <= pH_10_H))
    {
        pH_val = 10;
    }
    else if ((raw_data >= pH_11_L) && (raw_data <= pH_11_H))
    {
        pH_val = 11;
    }
    else if ((raw_data >= pH_12_L) && (raw_data <= pH_12_H))
    {
        pH_val = 12;
    }
    else if ((raw_data >= pH_13_L) && (raw_data <= pH_13_H))
    {
        pH_val = 13;
    }
    else if ((raw_data >= pH_14_L) && (raw_data <= pH_14_H))
    {
        pH_val = 14;
    }
    else
    {
        pH_val = pH_prev_val;
    }

    pH_prev_val = pH_val;
    pH_inf.pH = pH_val;
    set_pH_buff(pH_val);

    pH_inf.infection = infection_detect();
   
    printk("Raw data: %d || pH_val: %d \r\n", raw_data, pH_val);
    return pH_inf;
}

bool infection_detect(void)
{

    bool infection_status = false;
    static uint8_t ctr = 0;
    for (int i = 0; i < infection_detect_buff_size; i++)
    {
        /* This condition is necessary to initialize pH_infection_check buffer. Once all the values are set, the buffer then operates like a FIFO */
        if(pH_infection_check[i] == 0)
        {
            printk("Infection buff data is not ready\n\r");
            break;
        }

        else if (pH_infection_check[i] <= 7)
        {
            infection_status = false;
        }
        else 
        {
            ctr++;
            /* This counter checks if 3 out of 10 values have pH greater than 7. If  yes then report the infection. */
            if (ctr < 3)
            {
                infection_status = false;
            }
            else
            {
                ctr = 0;
                infection_status = true;
                break;
            }
            
        }
    }

    return infection_status;
}