/*
 * ads1118.h
 *
 *  Created on: Apr 26, 2021
 *      Author: LJH
 */

#ifndef INC_ADS1118_H_
#define INC_ADS1118_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "spi.h"
#include "sys.h"

// Config Register
//Operational status/single-shot conversion start
#define CONFIG_BIT_OS (1 << 15)
//MUX[2:0]: Input multiplexer configuration
#define CONFIG_BIT_MUX (7 << 12)
//PGA[2:0]: Programmable gain amplifier configuration
#define CONFIG_BIT_PGA (7 << 9)
//MODE: Device operating mode
#define CONFIG_BIT_MODE (1 << 8)
//DR[2:0]: Data rate
#define CONFIG_BIT_DR (7 << 5)
//TS_MODE: Temperature sensor mode
#define CONFIG_BIT_TS_MODE (1 << 4)
//PULL_UP_EN: Pull-up enable
#define CONFIG_BIT_PULLUP_EN (1 << 3)
//NOP: No operation
#define CONFIG_BIT_NOP (3 << 1)
//CONFIG_BIT_RESV: default
#define CONFIG_BIT_RESV (1 << 0)

#define ADS1118_CONST_6_144V_LSB_mV (0.1875)
#define ADS1118_CONST_4_096V_LSB_mV (0.125)
#define ADS1118_CONST_2_048V_LSB_mV (0.0625)
#define ADS1118_CONST_1_024V_LSB_mV (0.03125)
#define ADS1118_CONST_0_512V_LSB_mV (0.015625)
#define ADS1118_CONST_0_256V_LSB_mV (0.0078125)


typedef union
{
    struct
    {
        volatile unsigned char RESV : 1; //low
        volatile unsigned char NOP : 2;
        volatile unsigned char PULLUP : 1;
        volatile unsigned char TS_MODE : 1;
        volatile unsigned char DR : 3;
        volatile unsigned char MODE : 1;
        volatile unsigned char PGA : 3;
        volatile unsigned char MUX : 3;
        volatile unsigned char OS : 1; //high
    } stru;
    volatile unsigned int word;
    volatile unsigned char byte[2];
} ADS_InitTypeDef;

typedef enum
{
    CONVERING = 0x1,          //for read
    SINGLE_CONVER_START = 0x1 //for write
} ADS_OS_TypeDef;

typedef enum
{
    AINPN_0_1 = 0x0,
    AINPN_0_3 = 0x1,
    AINPN_1_3 = 0x2,
    AINPN_2_3 = 0x3,
    AINPN_0_GND = 0x4,
    AINPN_1_GND = 0x5,
    AINPN_2_GND = 0x6,
    AINPN_3_GND = 0x7
} ADS_MUX_TypeDef;

typedef enum
{
    PGA_6144 = 0x0,
    PGA_4096 = 0x1,
    PGA_2048 = 0x2,
    PGA_1024 = 0x3,
    PGA_512 = 0x4,
    PGA_256 = 0x5
} ADS_PGA_TypeDef;

typedef enum
{
    CONTIOUS = 0x0,
    SIGNLE_SHOT = 0x1
} ADS_MODE_TypeDef;

typedef enum
{
    DR_8_SPS = 0x0,
    DR_16_SPS = 0x1,
    DR_32_SPS = 0x2,
    DR_64_SPS = 0x3,
    DR_128_SPS = 0x4,
    DR_250_SPS = 0x5,
    DR_475_SPS = 0x6,
    DR_860_SPS = 0x7
} ADS_DATARATE_TypeDef;

typedef enum
{
    ADC_MODE = 0x0,
    TEMPERATURE_MODE = 0x1
} ADS_TSMODE_TypeDef;

typedef enum
{
    PULL_UP_DIS = 0x0,
    PULL_UP_EN = 0x1
} ADS_PULL_TypeDef;

typedef enum
{
    DATA_VALID = 0x1,
    DATA_INVALID = 0x2
} ADS_NOP_TypeDef;

typedef enum
{
    DATA_READY = 0x0,
    DATA_NREADY = 0x1
} ADS_RDY_TypeDef;


#define ADS1118_DISABLE HAL_GPIO_WritePin(AD_CS_GPIO_Port, AD_CS_Pin, GPIO_PIN_SET);
#define ADS1118_ENABLE  HAL_GPIO_WritePin(AD_CS_GPIO_Port, AD_CS_Pin, GPIO_PIN_RESET);

float ads1118_get_vol(uint8_t channel);


#ifdef __cplusplus
}
#endif

#endif /* INC_ADS1256_H_ */
