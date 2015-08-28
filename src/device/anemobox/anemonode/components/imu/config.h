/**
 * \file config.h
 * \brief Anemobox global definitions for the BSP Anemobox
 * \author fom
 * \version 1.2
 * \date 30 avril 2015
 *
 * 
 *
 */
#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus 
extern "C" {
#endif

#include <linux/types.h>
#include "mraa.h"
#include <sys/time.h>

/*unsigned integer types*/
typedef	uint8_t u8;/**< used for unsigned 8bit */
typedef	uint16_t u16;/**< used for unsigned 16bit */
typedef	uint32_t u32;/**< used for unsigned 32bit */
typedef	uint64_t u64;/**< used for unsigned 64bit */

/*signed integer types*/
typedef int8_t s8;/**< used for signed 8bit */
typedef	int16_t s16;/**< used for signed 16bit */
typedef	int32_t s32;/**< used for signed 32bit */
typedef	int64_t s64;/**< used for signed 64bit */

#define esc1 "\x1b[1;31m\x1b[10000;1f" 
#define esc2 "\x1b[0m"

#define SOME_TIME		usleep(100000)
#define MORE_TIME		usleep(1000000)

#define VERBOSE			0

#define FUNC_SUCCESS	0
#define FUNC_ERROR		1


mraa_result_t res, isr;

/*
* Used by init mraa lib for i2c context pointer
* Use the raw init /dev/i2c-1
*/
#define I2CBUS_ADAPTER	1
mraa_i2c_context i2c;

/*
* Used by init mraa lib for spi context pointer
*/
#define SPIBUS_ADAPTER	5
#define SPI_CS0_MRAA	23	

mraa_spi_context spi;
mraa_gpio_context spi_cs0;
#define fSCLK 10000000

/*
* Used by init mraa lib for pwm context pointer
*/
mraa_pwm_context pwm0;
mraa_pwm_context pwm1;
mraa_pwm_context pwm2;

#define LED_B_MRAA		0
#define LED_G_MRAA		14
#define LED_R_MRAA		20

/*
* Used by init mraa lib for user button context pointer
*/
#define RISING_EDGE				MRAA_GPIO_EDGE_RISING
#define FALLING_EDGE			MRAA_GPIO_EDGE_FALLING
#define USER_BTN		46
#define CAN_INT			33

mraa_gpio_context user_btn;

mraa_gpio_context can_int;

#ifdef __cplusplus
}
#endif

#endif