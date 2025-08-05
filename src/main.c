#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

#include "nau.h"
// LOG_MODULE_REGISTER(main_code, LOG_LEVEL_INF);

void phSensor_init()
{
    nau_init();
    k_msleep(500);
    // nau_offset_calib();
    // k_msleep(1000);
    nau_ctrl_reg(GAIN_1, VLDO_3_3, CR_10SPS, CALMOD_INT_OFFSET);
    k_msleep(500);
    nau_Read_ADC();
    k_msleep(500);
}


int main(void)
{
    // uint32_t raw_data = 0;
    // bool infection_status = false;
    pH_and_infection pH_inf;
    k_msleep(1000);
    // LOG_INF("Initiating pH Sensor ADC");
    phSensor_init();
    while (1) {
        while(!data_ready())
        {
            k_msleep(1);
        }
        // LOG_INF("Reading pH sensor");
        // raw_data = nau_Read_ADC();
        pH_inf = raw_data_to_pH();
        // printk("pH_val: %d || Avg_raw_data_5_val: %d", pH_val);
        // infection_status = infection_detect();
        if(pH_inf.infection == true)
        {
            printk("Infection_status = true\n\r");
        }
        else
        {
            printk("Infection_status = false\n\r");
        }
        k_msleep(1000);
    }
    return 0;
}