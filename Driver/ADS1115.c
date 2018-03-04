#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "stm32f10x.h"
#include "initialize.h"
#include "stm32f10x_conf.h"
#include "ADS1115.h"

extern void delay_ms(uint32_t x);

static void i2cStart(uint8_t address, uint8_t I2C_Direction){
    I2C_GenerateSTART(I2C1, ENABLE);
    while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) == ERROR); //等待占用总线
    /* 发送地址，要往这个地址写数据 */
    I2C_Send7bitAddress(I2C1, address, I2C_Direction);
}

static void i2cStop(void){
    I2C_GenerateSTOP(I2C1, ENABLE);
}

/**************************************************************************/
/*!
    @brief  Abstract away platform differences in Arduino wire library
*/
/**************************************************************************/
static uint8_t i2cread(void) {
    uint8_t tmp;
    /* 检测当前是否是主接收模式 */
    while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) == ERROR);
    tmp = I2C_ReceiveData(I2C1);
    while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) == ERROR);
    return tmp;
}

/**************************************************************************/
/*!
    @brief  Abstract away platform differences in Arduino wire library
*/
/**************************************************************************/
static void i2cwrite(uint8_t x) {
    /* 检测当前是否是主发送模式 */
    while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == ERROR);
    I2C_SendData(I2C1, x);
    while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) == ERROR);
}

/**************************************************************************/
/*!
    @brief  Writes 16-bits to the specified destination register
*/
/**************************************************************************/
static void writeRegister(uint8_t i2cAddress, uint8_t reg, uint16_t value) {
    i2cStart(i2cAddress, I2C_Direction_Transmitter);
    i2cwrite((uint8_t)reg);
    i2cwrite((uint8_t)(value>>8));
    i2cwrite((uint8_t)(value & 0xFF));
    i2cStop();
}

/**************************************************************************/
/*!
    @brief  Writes 16-bits to the specified destination register
*/
/**************************************************************************/
static uint16_t readRegister(uint8_t i2cAddress, uint8_t reg) {
    uint16_t result = 0;
    i2cStart(i2cAddress, I2C_Direction_Transmitter);
    i2cwrite(ADS1115_REG_POINTER_CONVERT);
    //重发启动信号，接收模式
    i2cStart(i2cAddress, I2C_Direction_Receiver);
    result = ((i2cread() << 8) | i2cread());
    i2cStop(); //关闭总线
    return result;
}


/**************************************************************************/
/*!
    @brief  Gets a single-ended ADC reading from the specified channel
*/
/**************************************************************************/
uint16_t readADC_SingleEnded(uint8_t channel) {
    // Start with default values
    uint16_t config = ADS1115_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
                    ADS1115_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
                    ADS1115_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1115_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1115_REG_CONFIG_DR_1600SPS   | // 1600 samples per second (default)
                    ADS1115_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)
    if (channel > 3) return 0;
    // Set PGA/voltage range
    config |= GAIN_TWOTHIRDS;

    // Set single-ended input channel
    switch (channel)
    {
    case (0):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_0;
      break;
    case (1):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_1;
      break;
    case (2):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_2;
      break;
    case (3):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_3;
      break;
    }

    // Set 'start single-conversion' bit
    config |= ADS1115_REG_CONFIG_OS_SINGLE;

    // Write config register to the ADC
    writeRegister(ADS1115_ADDRESS, ADS1115_REG_POINTER_CONFIG, config);

    // Wait for the conversion to complete
    delay_ms(ADS1115_CONVERSIONDELAY);

    // Read the conversion results
    // Shift 12-bit results right 4 bits for the ADS1115
    return readRegister(ADS1115_ADDRESS, ADS1115_REG_POINTER_CONVERT) >> 0;  
}

/**************************************************************************/
/*! 
    @brief  Reads the conversion results, measuring the voltage
            difference between the P (AIN0) and N (AIN1) input.  Generates
            a signed value since the difference can be either
            positive or negative.
*/
/**************************************************************************/
int16_t readADC_Differential_0_1() {
  // Start with default values
    uint16_t res;
    uint16_t config = ADS1115_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
                    ADS1115_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
                    ADS1115_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1115_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1115_REG_CONFIG_DR_1600SPS   | // 1600 samples per second (default)
                    ADS1115_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

    // Set PGA/voltage range
    config |= GAIN_TWOTHIRDS;
                    
    // Set channels
    config |= ADS1115_REG_CONFIG_MUX_DIFF_0_1;          // AIN0 = P, AIN1 = N

    // Set 'start single-conversion' bit
    config |= ADS1115_REG_CONFIG_OS_SINGLE;

    // Write config register to the ADC
    writeRegister(ADS1115_ADDRESS, ADS1115_REG_POINTER_CONFIG, config);

    // Wait for the conversion to complete
    delay_ms(ADS1115_CONVERSIONDELAY);

    // Read the conversion results
    res = readRegister(ADS1115_ADDRESS, ADS1115_REG_POINTER_CONVERT) >> 0;
    return (int16_t)res;
}

/**************************************************************************/
/*! 
    @brief  Reads the conversion results, measuring the voltage
            difference between the P (AIN2) and N (AIN3) input.  Generates
            a signed value since the difference can be either
            positive or negative.
*/
/**************************************************************************/
int16_t readADC_Differential_2_3() {
    // Start with default values
    uint16_t res;
    uint16_t config = ADS1115_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
                    ADS1115_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
                    ADS1115_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1115_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1115_REG_CONFIG_DR_1600SPS   | // 1600 samples per second (default)
                    ADS1115_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

    // Set PGA/voltage range
    config |= GAIN_TWOTHIRDS;

    // Set channels
    config |= ADS1115_REG_CONFIG_MUX_DIFF_2_3;          // AIN2 = P, AIN3 = N

    // Set 'start single-conversion' bit
    config |= ADS1115_REG_CONFIG_OS_SINGLE;

    // Write config register to the ADC
    writeRegister(ADS1115_ADDRESS, ADS1115_REG_POINTER_CONFIG, config);

    // Wait for the conversion to complete
    delay_ms(ADS1115_CONVERSIONDELAY);

    // Read the conversion results
    res = readRegister(ADS1115_ADDRESS, ADS1115_REG_POINTER_CONVERT) >> 0;
    return (int16_t)res;
}

/**************************************************************************/
/*!
    @brief  Sets up the comparator to operate in basic mode, causing the
            ALERT/RDY pin to assert (go from high to low) when the ADC
            value exceeds the specified threshold.

            This will also set the ADC in continuous conversion mode.
*/
/**************************************************************************/
void startComparator_SingleEnded(uint8_t channel, int16_t threshold)
{
    // Start with default values
    uint16_t config = ADS1115_REG_CONFIG_CQUE_1CONV   | // Comparator enabled and asserts on 1 match
                    ADS1115_REG_CONFIG_CLAT_LATCH   | // Latching mode
                    ADS1115_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1115_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1115_REG_CONFIG_DR_1600SPS   | // 1600 samples per second (default)
                    ADS1115_REG_CONFIG_MODE_CONTIN  | // Continuous conversion mode
                    ADS1115_REG_CONFIG_MODE_CONTIN;   // Continuous conversion mode

    // Set PGA/voltage range
    config |= GAIN_TWOTHIRDS;
                    
    // Set single-ended input channel
    switch (channel)
    {
    case (0):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_0;
      break;
    case (1):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_1;
      break;
    case (2):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_2;
      break;
    case (3):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_3;
      break;
    }

    // Set the high threshold register
    // Shift 12-bit results left 4 bits for the ADS1115
    writeRegister(ADS1115_ADDRESS, ADS1115_REG_POINTER_HITHRESH, threshold << 0);

    // Write config register to the ADC
    writeRegister(ADS1115_ADDRESS, ADS1115_REG_POINTER_CONFIG, config);
}

/**************************************************************************/
/*!
    @brief  In order to clear the comparator, we need to read the
            conversion results.  This function reads the last conversion
            results without changing the config value.
*/
/**************************************************************************/
int16_t getLastConversionResults(void)
{
    uint16_t res;
    // Wait for the conversion to complete
    delay_ms(ADS1115_CONVERSIONDELAY);

    // Read the conversion results
    res = readRegister(ADS1115_ADDRESS, ADS1115_REG_POINTER_CONVERT) >> 0;
    return (int16_t)res;
}
