/*
 * ads1256.h
 *
 *  Created on: Jan 20, 2021
 *      Author: LJH
 */

#ifndef INC_ADS1256_H_
#define INC_ADS1256_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "spi.h"
#include "sys.h"
extern int32_t adcRawVaule;
extern double voltage;
extern double filterVoltage;
extern double filterVoltage2;
/****************************REG**************/
#define STATUS  0x00
#define MUX     0x01
#define ADCON   0x02
#define DRATE   0x03
#define IO      0x04
#define OFC0    0x05
#define OFC1    0x06
#define OFC2    0x07
#define FSC0    0x08
#define FSC1    0x09
#define FSC2    0x0A

/****************************CMD*******************/
#define WAKEUP  0x00
#define RDATA   0x01
#define TDATAC  0x03
#define SDATAC  0x0F
#define RREG    0x10
#define WREG    0x50
#define SELFCAL 0xF0
#define SELFOCAL 0xF1
#define SELFGCAL 0xF2
#define SYSOCAL  0xF3
#define SYSGCAL  0xF4
#define SYNC     0xFC
#define STANDBY  0xFD
#define RESET    0xFE

/*****************************gain select*********************/
#define GAIN_1   0x00
#define GAIN_2   0x01
#define GAIN_4   0x02
#define GAIN_8   0x03
#define GAIN_16  0x04
#define GAIN_32  0x05
#define GAIN_64  0x06

/*****************************ads1256 simple rate select*******/
#define RATE_30000 0xF0
#define RATE_15000 0xE0
#define RATE_7500  0xD0
#define RATE_3750  0xC0
#define RATE_2000  0xB0
#define RATE_1000  0xA1
#define RATE_500   0x92
#define RATE_100   0x82
#define RATE_60    0x72
#define RATE_50    0x63
#define RATE_30    0x53
#define RATE_25    0x43
#define RATE_15    0x33
#define RATE_10    0x23
#define RATE_5     0x13
#define RATE_2_5   0x03

/*****************************ads1256 channel select**************/

#define MUXP_AIN0  0x00
#define MUXP_AIN1  0x10
#define MUXP_AIN2  0x20
#define MUXP_AIN3  0x30
#define MUXP_AIN4  0x40
#define MUXP_AIN5  0x50
#define MUXP_AIN6  0x60
#define MUXP_AIN7  0x70
#define MUXP_AINCOM 0x80

#define MUXN_AIN0   0x00
#define MUXN_AIN1   0x01
#define MUXN_AIN2   0x02
#define MUXN_AIN3   0x03
#define MUXN_AIN4   0x04
#define MUXN_AIN5   0x05
#define MUXN_AIN6   0x06
#define MUXN_AIN7   0x07
#define MUXN_AINCOM 0x08


double ads1256ReadValueV2(u8 channel, int buff_on,int filter);
void ads1256Init(u8 channel,int buff_on);

void TEST_ads1256_adc_manul_cal(void);
void TEST_ads1256_channnel(void);



#ifdef __cplusplus
}
#endif

#endif /* INC_ADS1256_H_ */
