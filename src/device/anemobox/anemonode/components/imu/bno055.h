/**
 * \file bno055.h
 * \brief Inertial station BNO055 definitions
 * \author fom
 * \version 1.3
 * \date 7 mai 2015
 *
 * Access function set for Inertial station BNO055 definitions
 *
 */
#ifndef BNO055_H
#define BNO055_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

#define PAGE_ID_0	0x00
#define PAGE_ID_1	0x01

#define BNO055_I2C_BASE_ADDR 	        0x29

#define BNO055_WHOAMI					0xA0

#define BNO055_SYS_TRIGGER				0x3F
#define BNO055_SYS_RST					(u8)(1<<5)

#define BNO055_ST_RESULT_ADDR			0x36			

#define BNO055_OPR_MODE_ADDR			0x3D
#define BNO055_OPR_MODE_CONFIG_MODE		0x00
#define BNO055_ACCONLY_OPR_MODE			0x01
#define BNO055_NDOF_OPR_MODE			0x0C
#define BNO055_NDOF_FMC_OFF_MODE		0x0B

#define SYS_STATUS_IDLE					0x00
#define SYS_STATUS_FUSION				0x05

#define BNO055_PWR_MODE_ADDR			0x3E
#define BNO055_PWR_MODE_NORMAL			0x00
#define BNO055_PWR_MODE_LOW_PWR			0x01

#define BNO055_SYS_ERR					0x3A
#define BNO055_SYS_STATUS				0x39

#define BNO055_UNIT_SEL_ADDR			0x3B

#define BNO055_ACC_MPSS_DIVIDER			(float)100.0	
#define BNO055_ACC_MG_DIVIDER			(float)1.0	

#define G_SCALE							(float)1000.0

#define BNO055_MAG_UT_DIVIDER			(float)16.0

#define BNO055_GYRO_DPS_DIVIDER			(float)16.0
#define BNO055_GYRO_RPS_DIVIDER         (float)900.0

#define BNO055_EULER_DIVIDER            (float)16.0

/***************************************************/
/**\name	REGISTER ADDRESS DEFINITION  */
/***************************************************/
/* Page id register definition*/
#define BNO055_PAGE_ID_ADDR				    0X07

/* PAGE0 REGISTER DEFINITION START*/
#define BNO055_CHIP_ID_ADDR                 0x00
#define BNO055_ACCEL_REV_ID_ADDR			0x01
#define BNO055_MAG_REV_ID_ADDR              0x02
#define BNO055_GYRO_REV_ID_ADDR             0x03
#define BNO055_SW_REV_ID_LSB_ADDR			0x04
#define BNO055_SW_REV_ID_MSB_ADDR			0x05
#define BNO055_BL_REV_ID_ADDR				0X06

/* Accel data register*/
#define BNO055_ACCEL_DATA_X_LSB_ADDR		0X08
#define BNO055_ACCEL_DATA_X_MSB_ADDR		0X09
#define BNO055_ACCEL_DATA_Y_LSB_ADDR		0X0A
#define BNO055_ACCEL_DATA_Y_MSB_ADDR		0X0B
#define BNO055_ACCEL_DATA_Z_LSB_ADDR		0X0C
#define BNO055_ACCEL_DATA_Z_MSB_ADDR		0X0D

/*Mag data register*/
#define BNO055_MAG_DATA_X_LSB_ADDR			0X0E
#define BNO055_MAG_DATA_X_MSB_ADDR			0X0F
#define BNO055_MAG_DATA_Y_LSB_ADDR			0X10
#define BNO055_MAG_DATA_Y_MSB_ADDR			0X11
#define BNO055_MAG_DATA_Z_LSB_ADDR			0X12
#define BNO055_MAG_DATA_Z_MSB_ADDR			0X13

/*Gyro data registers*/
#define BNO055_GYRO_DATA_X_LSB_ADDR			0X14
#define BNO055_GYRO_DATA_X_MSB_ADDR			0X15
#define BNO055_GYRO_DATA_Y_LSB_ADDR			0X16
#define BNO055_GYRO_DATA_Y_MSB_ADDR			0X17
#define BNO055_GYRO_DATA_Z_LSB_ADDR			0X18
#define BNO055_GYRO_DATA_Z_MSB_ADDR			0X19

/*Quaternion data registers*/
#define BNO055_QUATERNION_DATA_W_LSB_ADDR	0X20
#define BNO055_QUATERNION_DATA_W_MSB_ADDR	0X21
#define BNO055_QUATERNION_DATA_X_LSB_ADDR	0X22
#define BNO055_QUATERNION_DATA_X_MSB_ADDR	0X23
#define BNO055_QUATERNION_DATA_Y_LSB_ADDR	0X24
#define BNO055_QUATERNION_DATA_Y_MSB_ADDR	0X25
#define BNO055_QUATERNION_DATA_Z_LSB_ADDR	0X26
#define BNO055_QUATERNION_DATA_Z_MSB_ADDR	0X27

/* Accel offset register*/
#define BNO055_ACCEL_OFFSET_X_LSB_ADDR		0X55
#define BNO055_ACCEL_OFFSET_X_MSB_ADDR		0X56
#define BNO055_ACCEL_OFFSET_Y_LSB_ADDR		0X57
#define BNO055_ACCEL_OFFSET_Y_MSB_ADDR		0X58
#define BNO055_ACCEL_OFFSET_Z_LSB_ADDR		0X59
#define BNO055_ACCEL_OFFSET_Z_MSB_ADDR		0X5A

/*Mag offset register*/
#define BNO055_MAG_OFFSET_X_LSB_ADDR		0X5B
#define BNO055_MAG_OFFSET_X_MSB_ADDR		0X5C
#define BNO055_MAG_OFFSET_Y_LSB_ADDR		0X5D
#define BNO055_MAG_OFFSET_Y_MSB_ADDR		0X5E
#define BNO055_MAG_OFFSET_Z_LSB_ADDR		0X5F
#define BNO055_MAG_OFFSET_Z_MSB_ADDR		0X60

/*Gyro offset registers*/
#define BNO055_GYRO_OFFSET_X_LSB_ADDR		0X61
#define BNO055_GYRO_OFFSET_X_MSB_ADDR		0X62
#define BNO055_GYRO_OFFSET_Y_LSB_ADDR		0X63
#define BNO055_GYRO_OFFSET_Y_MSB_ADDR		0X64
#define BNO055_GYRO_OFFSET_Z_LSB_ADDR		0X65
#define BNO055_GYRO_OFFSET_Z_MSB_ADDR		0X66

#define ACCEL_REG_SIZE						6
#define MAG_REG_SIZE						6
#define GYRO_REG_SIZE						6
#define QUATERNION_REG_SIZE                 8
#define EULER_REG_SIZE                      6

/* Euler data registers */
#define BNO055_EULER_H_LSB_ADDR             0X1A
#define BNO055_EULER_H_MSB_ADDR             0X1B
#define BNO055_EULER_R_LSB_ADDR             0X1C
#define BNO055_EULER_R_MSB_ADDR             0X1D
#define BNO055_EULER_P_LSB_ADDR             0X1E
#define BNO055_EULER_P_MSB_ADDR             0X1F

/*!
* \brief struct for Accel data read from registers
*/
struct BNO055_accel_t
{
	s16 x;/**< accel x data */
	s16 y;/**< accel y data */
	s16 z;/**< accel z data */
};

/*!
* \brief struct for Mag data read from registers
*/
struct BNO055_mag_t
{
	s16 x;/**< mag x data */
	s16 y;/**< mag y data */
	s16 z;/**< mag z data */
};

/*!
* \brief struct for Gyro data read from registers
*/
struct BNO055_gyro_t
{
	s16 x;/**< gyro x data */
	s16 y;/**< gyro y data */
	s16 z;/**< gyro z data */
};

/*!
* \brief struct for Quaternion data read from registers
*/
struct BNO055_quaternion_t
{
	s16 w;/**< Quaternion w data */
	s16 x;/**< Quaternion x data */
	s16 y;/**< Quaternion y data */
	s16 z;/**< Quaternion z data */
};

/*!
* \brief struct for Accel bias read from registers
*/
struct BNO055_accel_bias_t
{
	s32 x;/**< accel x data bias */
	s32 y;/**< accel y data bias */
	s32 z;/**< accel z data bias */
};

/*!
* \brief struct for Mag data read from registers
*/
struct BNO055_mag_bias_t
{
	s32 x;/**< mag x data bias*/
	s32 y;/**< mag y data bias*/
	s32 z;/**< mag z data bias*/
};

/*!
* \brief struct for Gyro data read from registers
*/
struct BNO055_gyro_bias_t
{
    s32 x;/**< gyro x data bias*/
    s32 y;/**< gyro y data bias*/
    s32 z;/**< gyro z data bias*/
};

/*!
* \brief struct for Euler data read from registers
*/
struct BNO055_euler_bias_t
{
    s32 h;/**< gyro x data bias*/
    s32 r;/**< gyro y data bias*/
    s32 p;/**< gyro z data bias*/
};

/*!
* \brief struct for Accel read from registers as float data
*/
struct BNO055_accel_float_t
{
	float x;/**< accel x data as float*/
	float y;/**< accel y data as float*/
	float z;/**< accel z data as float*/
};
/*!
* \brief struct for Mag data read from registers as float
*/
struct BNO055_mag_float_t
{
	float x;/**< mag x data as float*/
	float y;/**< mag y data as float*/
	float z;/**< mag z data as float*/
};
/*!
* \brief struct for Gyro data read from registers as float
*/
struct BNO055_gyro_float_t
{
	float x;/**< gyro x data as float*/
	float y;/**< gyro y data as float*/
	float z;/**< gyro z data as float*/
};
/*!
* \brief struct for Quaternion data read from registers as float
*/
struct BNO055_quaternion_float_t
{
    float w;/**< Quaternion w data as float*/
    float x;/**< Quaternion x data as float*/
    float y;/**< Quaternion y data as float*/
    float z;/**< Quaternion z data as float*/
};
/*!
* \brief struct for Quaternion data read from registers as float
*/
struct BNO055_euler_float_t
{
    float h;/**< Euler h data as float*/
    float r;/**< Euler r data as float*/
    float p;/**< Euler p data as float*/
};

struct BNO055_accel_float_t bno055_accel_out;
struct BNO055_mag_float_t bno055_mag_out;
struct BNO055_gyro_float_t bno055_gyro_out;
struct BNO055_quaternion_float_t bno055_quaternion_out;
struct BNO055_euler_float_t bno055_euler_out;

struct BNO055_accel_bias_t accel_bias;
struct BNO055_mag_bias_t mag_bias;
struct BNO055_gyro_bias_t gyro_bias;
struct BNO055_euler_bias_t euler_bias;

/*! \fn void LPS331_readFloatTemperaturePressure(struct LPS331_presstemp_float_t* out)
    \brief Read the temperature and pressure and perform the conversion in float
    \param struct to hold the data as float values
    \return None
*/
void sig_handler(int signum);

/*! \fn int BNO055_initHW()
    \brief Inertial station HW init
    \param None
    \return None
*/
int BNO055_initHW();

/*! \fn int BNO055_reset()
    \brief Inertial station HW reset
    \param None
    \return 0 for good and other values for failure
*/
int BNO055_reset();

/*! \fn int BNO055_getStatus()
    \brief Get status from the register
    \param None
    \return None
*/
int BNO055_getStatus();

/*! \fn int setMode()
    \brief Set operation mode to normal
    \param None
    \return None
*/
void setMode(u8 mode);

/*! \fn void BNO055_getAccel(struct BNO055_accel_float_t* out)
    \brief Calibration function for the accelerometer
    \param struct to hold the bias data 
    \return None
*/
void BNO055_accel_calib(struct BNO055_accel_bias_t* accel_bias);

/*! \fn void BNO055_getAccel(struct BNO055_accel_float_t* out)
    \brief Read the raw acceleration without compenstation
    \param struct to hold the data 
    \return None
*/
void BNO055_getAccel(struct BNO055_accel_float_t* out);

/*! \fn void BNO055_getCompensatedAccel(struct BNO055_accel_float_t* out)
    \brief Read the raw acceleration with compenstation
    \param struct to hold the data as float values
    \return None
*/
void BNO055_getCompensatedAccel(struct BNO055_accel_float_t* out);

/*! \fn void BNO055_getMag(struct BNO055_mag_float_t* out)
    \brief Calibration function for the magnetometer
    \param struct to hold the bias data 
    \return None
*/
void BNO055_mag_calib(struct BNO055_mag_bias_t* mag_bias);

/*! \fn void BNO055_getCompensatedMag(struct BNO055_mag_float_t* out)
    \brief Read the raw magnetometer data without compenstation
    \param struct to hold the data as float values
    \return None
*/
void BNO055_getMag(struct BNO055_mag_float_t* out);

/*! \fn void BNO055_getCompensatedMag(struct BNO055_mag_float_t* out)
    \brief Read the raw magnetometer data with compenstation
    \param struct to hold the data as float values
    \return None
*/
void BNO055_getCompensatedMag(struct BNO055_mag_float_t* out);

/*! \fn void BNO055_gyro_calib(struct BNO055_gyro_bias_t* gyro_bias)
    \brief Calibration function for the gyroscope
    \param struct to hold the data as float values
    \return None
*/
void BNO055_gyro_calib(struct BNO055_gyro_bias_t* gyro_bias);

/*! \fn void BNO055_getCompensatedGyro(struct BNO055_gyro_float_t* out)
    \brief Read the raw magnetometer data without compenstation
    \param struct to hold the data as float values
    \return None
*/
void BNO055_getGyro(struct BNO055_gyro_float_t* out);

/*! \fn void BNO055_getCompensatedGyro(struct BNO055_gyro_float_t* out)
    \brief Read the raw magnetometer data with compenstation
    \param struct to hold the data as float values
    \return None
*/
void BNO055_getCompensatedGyro(struct BNO055_gyro_float_t* out);

/*! \fn voidBNO055_getQuaternion(struct BNO055_quaternion_float_t* out)
    \brief Read the quaternion
    \param struct to hold the data as float values
    \return None
*/
void BNO055_getQuaternion(struct BNO055_quaternion_float_t* out);

/*! \fn voidBNO055_getEuler(struct BNO055_euler_float_t* out)
    \brief Read the quaternion
    \param struct to hold the data as float values
    \return None
*/
void BNO055_getEuler(struct BNO055_euler_float_t* out);

#ifdef __cplusplus
}
#endif

#endif