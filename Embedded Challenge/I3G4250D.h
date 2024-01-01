#ifndef __I3G4250D_H
#define __I3G4250D_H

#include<stdint.h>
#include<mbed.h>

/*Calibration*/
#define X_base 11
#define Y_base -3
#define Z_base -20

/*register address*/
#define I3G4250D_WHO_AM_I_ADDR          0x0F  /* device identification register */
#define I3G4250D_CTRL_REG1_ADDR         0x20  /* Control register 1 */
#define I3G4250D_CTRL_REG2_ADDR         0x21  /* Control register 2 */
#define I3G4250D_CTRL_REG3_ADDR         0x22  /* Control register 3 */
#define I3G4250D_CTRL_REG4_ADDR         0x23  /* Control register 4 */
#define I3G4250D_CTRL_REG5_ADDR         0x24  /* Control register 5 */

#define I3G4250D_OUT_X_L_ADDR           0x28  /* Output Register X */
#define I3G4250D_OUT_X_H_ADDR           0x29  /* Output Register X */
#define I3G4250D_OUT_Y_L_ADDR           0x2A  /* Output Register Y */
#define I3G4250D_OUT_Y_H_ADDR           0x2B  /* Output Register Y */
#define I3G4250D_OUT_Z_L_ADDR           0x2C  /* Output Register Z */
#define I3G4250D_OUT_Z_H_ADDR           0x2D  /* Output Register Z */

/* cmd*/
#define READ_CMD 0x80
#define MULTI_BYTE_CMD 0x40
#define SPI_DELAY 0.003

/** @defgroup Full_Scale_Sensitivity Full Scale Sensitivity
  * @{
  */
#define I3G4250D_SENSITIVITY_245DPS  ((float)8.75f)         /*!< gyroscope sensitivity with 250 dps full scale [DPS/LSB]  */
#define I3G4250D_SENSITIVITY_500DPS  ((float)17.50f)        /*!< gyroscope sensitivity with 500 dps full scale [DPS/LSB]  */
#define I3G4250D_SENSITIVITY_2000DPS ((float)70.00f)        /*!< gyroscope sensitivity with 2000 dps full scale [DPS/LSB] */

/** @defgroup OutPut_DataRate_Selection OutPut DataRate Selection
  * @{
  */
#define I3G4250D_OUTPUT_DATARATE_1    ((uint8_t)0x00)
#define I3G4250D_OUTPUT_DATARATE_2    ((uint8_t)0x40)
#define I3G4250D_OUTPUT_DATARATE_3    ((uint8_t)0x80)
#define I3G4250D_OUTPUT_DATARATE_4    ((uint8_t)0xC0)

/** @defgroup Bandwidth_Selection Bandwidth Selection
  * @{
  */
#define I3G4250D_BANDWIDTH_1         ((uint8_t)0x00)
#define I3G4250D_BANDWIDTH_2         ((uint8_t)0x10)
#define I3G4250D_BANDWIDTH_3         ((uint8_t)0x20)
#define I3G4250D_BANDWIDTH_4         ((uint8_t)0x30)

/** @defgroup Axes_Selection Axes Selection
  * @{
  */
#define I3G4250D_X_ENABLE            ((uint8_t)0x01)
#define I3G4250D_Y_ENABLE            ((uint8_t)0x02)
#define I3G4250D_Z_ENABLE            ((uint8_t)0x04)
#define I3G4250D_AXES_ENABLE         ((uint8_t)0x07)

/** @defgroup Power_Mode_selection Power Mode selection
  * @{
  */
#define I3G4250D_MODE_POWERDOWN       ((uint8_t)0x00)
#define I3G4250D_MODE_ACTIVE          ((uint8_t)0x08)

/** @defgroup High_Pass_Filter_Mode High Pass Filter Mode
  * @{
  */
#define I3G4250D_HPM_NORMAL_MODE_RES         ((uint8_t)0x00)
#define I3G4250D_HPM_REF_SIGNAL              ((uint8_t)0x10)
#define I3G4250D_HPM_NORMAL_MODE             ((uint8_t)0x20)
#define I3G4250D_HPM_AUTORESET_INT           ((uint8_t)0x30)

/** @defgroup High_Pass_CUT OFF_Frequency High Pass CUT OFF Frequency
  * @{
  */
#define I3G4250D_HPFCF_0              0x00
#define I3G4250D_HPFCF_1              0x01
#define I3G4250D_HPFCF_2              0x02
#define I3G4250D_HPFCF_3              0x03
#define I3G4250D_HPFCF_4              0x04
#define I3G4250D_HPFCF_5              0x05
#define I3G4250D_HPFCF_6              0x06
#define I3G4250D_HPFCF_7              0x07
#define I3G4250D_HPFCF_8              0x08
#define I3G4250D_HPFCF_9              0x09

/** @defgroup Endian_Data_selection Endian Data selection
  * @{
  */
#define I3G4250D_BLE_LSB                     ((uint8_t)0x00)
#define I3G4250D_BLE_MSB                     ((uint8_t)0x40)

/** @defgroup Full_Scale_Selection Full Scale Selection
  * @{
  */
#define I3G4250D_FULLSCALE_245       ((uint8_t)0x00)
#define I3G4250D_FULLSCALE_500       ((uint8_t)0x10)
#define I3G4250D_FULLSCALE_2000      ((uint8_t)0x20)
#define I3G4250D_FULLSCALE_SELECTION ((uint8_t)0x30)

/** @defgroup High_Pass_Filter_status High Pass Filter status
  * @{
  */
#define I3G4250D_HIGHPASSFILTER_DISABLE      ((uint8_t)0x00)
#define I3G4250D_HIGHPASSFILTER_ENABLE       ((uint8_t)0x10)

/* functions*/
uint16_t ReadRegister(SPI &spi, DigitalOut &CS, uint16_t address);

void WriteRegister(SPI &spi, DigitalOut &CS, uint16_t address, uint16_t config);

uint16_t ReadTwoRegister(SPI &spi, DigitalOut &CS, uint16_t base_address);

void ReadXYZ(SPI &spi, DigitalOut &CS, float *xyz);

int Init(SPI &spi, DigitalOut &CS);

#endif