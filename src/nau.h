// Registers
#define NAU_PU_CTRL 0x00
#define NAU_CTRL1 0x01
#define NAU_CTRL2 0x02

#define NAU_I2C_CTRL 0x11
#define NAU_ADCO_B2 0x12
#define NAU_ADCO_B1 0x13
#define NAU_ADCO_B0 0x14

#define NAU_OTP_B1 0x15
#define NAU_OTP_B0 0x16
#define NAU_PGA 0x1B
#define NAU7802_POWER_CAP 0x1C

#define NAU_DEV_CODE 0x1F

// Reg bit values
// CTRL1
#define VLDO_2_4 0x07
#define VLDO_2_7 0x06
#define VLDO_3_0 0x05
#define VLDO_3_3 0x04
#define VLDO_3_6 0x03
#define VLDO_3_9 0x02
#define VLDO_4_2 0x01
#define VLDO_4_5 0x00

#define GAIN_128 0x07
#define GAIN_64 0x06
#define GAIN_32 0x05
#define GAIN_16 0x04
#define GAIN_8 0x03
#define GAIN_4 0x02
#define GAIN_2 0x01
#define GAIN_1 0x00

// CTRL2
#define CR_320SPS 0x07
#define CR_80SPS 0x03
#define CR_40SPS 0x02
#define CR_20SPS 0x01
#define CR_10SPS 0x00

#define CALMOD_GAIN 0x03
#define CALMOD_EXT_OFFSET 0x02
#define CALMOD_INT_OFFSET 0x00

#define pH_1_L 355555
#define pH_1_H 575555

#define pH_2_L 215599
#define pH_2_H 255555

#define pH_3_L 132233
#define pH_3_H 178899

#define pH_4_L 55332
#define pH_4_H 99887

#define pH_5_L -7000
#define pH_5_H 32000

#define pH_6_L -61000
#define pH_6_H -13000

#define pH_7_L -94000
#define pH_7_H -65000

#define pH_8_L -139000
#define pH_8_H -102000

#define pH_9_L -180000
#define pH_9_H -144000

#define pH_10_L -250000
#define pH_10_H -230000

#define pH_11_L -290000
#define pH_11_H -260000

#define pH_12_L -350000
#define pH_12_H -320000

#define pH_13_L -400000
#define pH_13_H -380000

#define pH_14_L -480000
#define pH_14_H -420000


typedef struct pH_and_infection {
    uint8_t pH;
    bool infection;
}pH_and_infection;


// Functions
bool nau_init();
void nau_offset_calib();
void nau_ctrl_reg(uint8_t gain, uint8_t VLDO, uint8_t CR, uint8_t CALMOD);
uint32_t nau_Read_ADC();
bool data_ready();
struct pH_and_infection raw_data_to_pH(void);
bool infection_detect(void);

