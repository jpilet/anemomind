/**
 * \file bno055.c
 * \brief Inertial station BNO055 implementations
 * \author fom
 * \version 1.3
 * \date 7 mai 2015
 *
 * Access function set for Inertial station BNO055 implementations
 *
 */
#include "bno055.h"

void sig_handler(int signum)
{
    printf("Received signal %d : Stop i2c context.\n", signum);
    //mraa_i2c_stop(i2c);
    exit(-1);
}

int BNO055_getStatus()
{
	u8 status;
	i2c = mraa_i2c_init_raw(I2CBUS_ADAPTER);

	mraa_i2c_address(i2c, BNO055_I2C_BASE_ADDR);
	status = mraa_i2c_read_byte_data(i2c, BNO055_SYS_STATUS);
/*	if(status == 0)
	{
		res = mraa_i2c_write_byte_data(i2c, BNO055_NDOF_OPR_MODE, BNO055_OPR_MODE_ADDR);
		usleep(25000);
	}*/
	return status;
}

int BNO055_initHW()
{
	u8 status, error;
	u8 res;
	if(VERBOSE)
	{
		printf("BNO055 Hardware Init\n");
	}


	i2c = mraa_i2c_init_raw(I2CBUS_ADAPTER);

	mraa_i2c_address(i2c, BNO055_I2C_BASE_ADDR);
	error = mraa_i2c_read_byte_data(i2c, BNO055_SYS_ERR);

	mraa_i2c_address(i2c, BNO055_I2C_BASE_ADDR);
	status = mraa_i2c_read_byte_data(i2c, BNO055_SYS_STATUS);
	
	mraa_i2c_address(i2c, BNO055_I2C_BASE_ADDR);
	res = mraa_i2c_read_byte_data(i2c, BNO055_CHIP_ID_ADDR);

	printf("Init checking Status : %u - Message %u\n", status, error);
	if(res != BNO055_WHOAMI)	
	{	
		printf("Chip ID error ..., content is : %d \n", res);	
	}
	else
	{
		if(1)
		{
			printf("BNO055 WAI register content : %#X\n", res);
		}	

		res = mraa_i2c_write_byte_data(i2c, BNO055_PWR_MODE_NORMAL, BNO055_PWR_MODE_ADDR);
		usleep(25000);

		res = mraa_i2c_write_byte_data(i2c, BNO055_NDOF_OPR_MODE, BNO055_OPR_MODE_ADDR);
		usleep(25000);

		status = mraa_i2c_read_byte_data(i2c, BNO055_SYS_STATUS);

		error = mraa_i2c_read_byte_data(i2c, BNO055_SYS_ERR);
		printf("Init checking Status : %u - Message %u\n", status, error);

		error = mraa_i2c_read_byte_data(i2c, BNO055_ST_RESULT_ADDR);

		if(1)
		{
			printf("Status result : %d\n", error);
			if(error & 0x01)
			{
				printf("Accelerometer self test success\n");
			}
			else
			{
				printf("Accelerometer self test success\n");
			}

			if(error & 0x02)
			{
				printf("Magnetometer self test success\n");
			}
			else
			{
				printf("Magnetometer self test success\n");
			}

			if(error & 0x04)
			{
				printf("Gyroscope self test success\n");
			}
			else
			{
				printf("Gyroscope self test success\n");
			}

			if(error & 0x08)
			{
				printf("MCU self test success\n");
			}
			else
			{
				printf("MCU self test success\n");
			}
		}
	}
	mraa_i2c_stop(i2c);
	return res;
}

void setMode(u8 mode)
{
	//i2c = mraa_i2c_init_raw(I2CBUS_ADAPTER);
	res = mraa_i2c_address(i2c, BNO055_I2C_BASE_ADDR);
	mraa_i2c_write_byte_data(i2c, BNO055_OPR_MODE_CONFIG_MODE, BNO055_OPR_MODE_ADDR);
	mraa_i2c_write_byte_data(i2c, mode, BNO055_OPR_MODE_ADDR);
	//mraa_i2c_stop(i2c);
	usleep(650000);
}

int BNO055_reset()
{
	u8 status;
	mraa_init();
	i2c = mraa_i2c_init_raw(I2CBUS_ADAPTER);
	
	
	//mraa_i2c_frequency(i2c, MRAA_I2C_FAST);
	res = mraa_i2c_address(i2c, BNO055_I2C_BASE_ADDR);

	const int MAX_TRY = 100000;
	int try;
	uint8_t sys_status;

	// Switch to CONFIG_MODE
	try = 0;
	for (;;)
	{
		res = mraa_i2c_address(i2c, BNO055_I2C_BASE_ADDR);
		status = mraa_i2c_write_byte_data(i2c, BNO055_OPR_MODE_CONFIG_MODE, BNO055_OPR_MODE_ADDR);
		if (status != 0)
		{
			return status;
		}
		res = mraa_i2c_address(i2c, BNO055_I2C_BASE_ADDR);
		status = mraa_i2c_write_byte_data(i2c, PAGE_ID_0, BNO055_PAGE_ID_ADDR);
		printf("%s %d\n", "Writing page : I2C status : ", status);
		if (status == 0)
		{
			printf("try line %d : %d\n", __LINE__, try);
			break;
		}

		if (++try >= MAX_TRY)
		{
			return -1;
		}
	}

	res = mraa_i2c_address(i2c, BNO055_I2C_BASE_ADDR);
	status = mraa_i2c_write_byte_data(i2c, BNO055_OPR_MODE_CONFIG_MODE, BNO055_OPR_MODE_ADDR);
	if (status != 0)
	{
		return status;
	}

	usleep(650000);

	// Software reset
	status = mraa_i2c_write_byte_data(i2c, BNO055_SYS_RST, BNO055_SYS_TRIGGER);
	if (status != 0)
	{
		return status;
	}

	usleep(650000);
	mraa_i2c_stop(i2c);
	i2c = mraa_i2c_init_raw(I2CBUS_ADAPTER);
	// Wait for the POST to finish
	try = 0;
	for (;;)
	{
		sys_status = mraa_i2c_read_byte_data(i2c, BNO055_SYS_STATUS);
		usleep(100000);
		printf("%s\n", "Reading status register");
		if (sys_status == SYS_STATUS_IDLE)
		{
			printf("try line %d : %d\n", __LINE__, try);
			break;
		}

		if (++try >= MAX_TRY)
		{
			return -1;
		}

		usleep(100000);
	}

	status = mraa_i2c_write_byte_data(i2c, BNO055_PWR_MODE_NORMAL, BNO055_PWR_MODE_ADDR);

	if (status != 0)
	{
		return status;
	}

	// Set running mode
	status = mraa_i2c_write_byte_data(i2c, BNO055_NDOF_OPR_MODE, BNO055_OPR_MODE_ADDR);
	if (status != 0)
	{
		return status;
	}

	try = 0;
	for (;;)
	{
		usleep(100000);

		sys_status = mraa_i2c_read_byte_data(i2c, BNO055_SYS_STATUS);
		if (status != 0)
		{
			return status;
		}

		if (sys_status == SYS_STATUS_FUSION)
		{
			printf("try line %d : %d\n", __LINE__, try);
			status = mraa_i2c_write_byte_data(i2c, BNO055_PWR_MODE_NORMAL, BNO055_PWR_MODE_ADDR);

			if (status != 0)
			{
				return status;
			}

			// Set running mode
			status = mraa_i2c_write_byte_data(i2c, BNO055_NDOF_OPR_MODE, BNO055_OPR_MODE_ADDR);
			if (status != 0)
			{
				return status;
			}
			break;
		}

		if (++try >= MAX_TRY)
		{
			return -1;
		}
	}
	mraa_i2c_stop(i2c);
	return status;
}


void BNO055_accel_calib(struct BNO055_accel_bias_t* accel_bias)
{
	u8 mode;
	u8 accel_data_out[ACCEL_REG_SIZE];
	printf("Accelerometer Calibration: Put device on a level surface and keep motionless! Wait......\n");

	u16 sample_count = 256, i;
	struct BNO055_accel_t accel_tmp;

	i2c = mraa_i2c_init_raw(I2CBUS_ADAPTER);
	mraa_i2c_address(i2c, BNO055_I2C_BASE_ADDR);
	mode = mraa_i2c_read_byte_data(i2c, BNO055_OPR_MODE_ADDR);
	if(!(mode == BNO055_NDOF_FMC_OFF_MODE || mode == BNO055_NDOF_OPR_MODE))
	{
		printf("False mode\n");
	}
	accel_bias->x = 0;
	accel_bias->y = 0;
	accel_bias->z = 0;

	for(i = 0; i < sample_count; i++)
	{
		mraa_i2c_read_bytes_data(i2c, BNO055_ACCEL_DATA_X_LSB_ADDR, accel_data_out, ACCEL_REG_SIZE);	
		accel_tmp.x = (s16)((s16)(accel_data_out[1]<<8)) | accel_data_out[0];
		accel_tmp.y = (s16)((s16)(accel_data_out[3]<<8)) | accel_data_out[2];
		accel_tmp.z = (s16)((s16)(accel_data_out[5]<<8)) | accel_data_out[4];

		accel_bias->x += (s32)(accel_tmp.x);
		accel_bias->y += (s32)(accel_tmp.y);
		accel_bias->z += (s32)(accel_tmp.z);

		usleep(20000);
	}
	accel_bias->x /= ((s32)sample_count);
	accel_bias->y /= ((s32)sample_count);
	accel_bias->z /= ((s32)sample_count);
	if(VERBOSE)
	{
		fprintf(stderr, esc1"Accelerometer bias output\t:\t%f \t%f \t%f\n"esc2, (float)accel_bias->x, (float)accel_bias->y, (float)accel_bias->z);
	}
	// Remove gravity from the z-axis accelerometer bias calculation
	if(accel_bias->z > 0)
	{
		accel_bias->z -= (s32) 1000;
	}
  	else
  	{
  		accel_bias->z += (s32) 1000;
  	}

  	mraa_i2c_address(i2c, BNO055_I2C_BASE_ADDR);
  	mraa_i2c_write_byte_data(i2c, BNO055_ACCEL_OFFSET_X_LSB_ADDR, (s16)accel_bias->x & 0xFF);
  	mraa_i2c_write_byte_data(i2c, BNO055_ACCEL_OFFSET_X_MSB_ADDR, ((s16)accel_bias->x >> 8) & 0xFF);
  	mraa_i2c_write_byte_data(i2c, BNO055_ACCEL_OFFSET_Y_LSB_ADDR, (s16)accel_bias->y & 0xFF);
  	mraa_i2c_write_byte_data(i2c, BNO055_ACCEL_OFFSET_Y_MSB_ADDR, ((s16)accel_bias->y >> 8) & 0xFF);
  	mraa_i2c_write_byte_data(i2c, BNO055_ACCEL_OFFSET_Z_LSB_ADDR, (s16)accel_bias->z & 0xFF);
  	mraa_i2c_write_byte_data(i2c, BNO055_ACCEL_OFFSET_Z_MSB_ADDR, ((s16)accel_bias->z >> 8) & 0xFF);
  	mraa_i2c_stop(i2c);
}

void BNO055_getCompensatedAccel(struct BNO055_accel_float_t* out)
{
	out->x = out->x-(float)(accel_bias.x)/G_SCALE;
	out->y = out->y-(float)(accel_bias.y)/G_SCALE;
	out->z = out->z-(float)(accel_bias.z)/G_SCALE;
}

void BNO055_getAccel(struct BNO055_accel_float_t* out)
{
	u8 accel_data_out[ACCEL_REG_SIZE];
	int status, error;
	s16 x;
	s16 y;
	s16 z;

	i2c = mraa_i2c_init_raw(I2CBUS_ADAPTER);
	mraa_i2c_address(i2c, BNO055_I2C_BASE_ADDR);
	mraa_i2c_read_bytes_data(i2c, BNO055_ACCEL_DATA_X_LSB_ADDR, accel_data_out, ACCEL_REG_SIZE);

	status = mraa_i2c_read_byte_data(i2c, BNO055_SYS_STATUS);

	error = mraa_i2c_read_byte_data(i2c, BNO055_SYS_ERR);
	printf("Status : %u - Message %u\n", status, error);
	printf("BNO055 get accelerometer data...\n");
	if(VERBOSE)
	{	
		if(i2c != NULL)
		{
			printf("%s\n", "i2c context pointer initialized");
			printf("%p\n", i2c);
		}
	}

	mraa_i2c_stop(i2c);

	x = (s16)(accel_data_out[1]<<8) | accel_data_out[0];
	y = (s16)(accel_data_out[3]<<8) | accel_data_out[2];
	z = (s16)(accel_data_out[5]<<8) | accel_data_out[4];

	out->x = ((float)(x)/BNO055_ACC_MG_DIVIDER)/G_SCALE;
	out->y = ((float)(y)/BNO055_ACC_MG_DIVIDER)/G_SCALE;
	out->z = ((float)(z)/BNO055_ACC_MG_DIVIDER)/G_SCALE;
}


void BNO055_mag_calib(struct BNO055_mag_bias_t* mag_bias)
{
	u8 mode, j;
	u8 mag_data_out[MAG_REG_SIZE];
	s16 mag_min[MAG_REG_SIZE] = {0, 0, 0};
	s16 mag_max[MAG_REG_SIZE] = {0, 0, 0};	
	s16 mag_temp[MAG_REG_SIZE] = {0, 0, 0};
	u16 sample_count = 256, i;
	struct BNO055_mag_t mag_tmp;

	printf("Mag Calibration: Wave device in a figure eight until done! Wait......\n");
	i2c = mraa_i2c_init_raw(I2CBUS_ADAPTER);
	mraa_i2c_address(i2c, BNO055_I2C_BASE_ADDR);
	mode = mraa_i2c_read_byte_data(i2c, BNO055_OPR_MODE_ADDR);
	if(!(mode == BNO055_NDOF_FMC_OFF_MODE || mode == BNO055_NDOF_OPR_MODE))
	{
		printf("False mode\n");
	}
	mag_bias->x = 0;
	mag_bias->y = 0;
	mag_bias->z = 0;

	for(i = 0; i < sample_count; i++)
	{
		mraa_i2c_read_bytes_data(i2c, BNO055_ACCEL_DATA_X_LSB_ADDR, mag_data_out, ACCEL_REG_SIZE);	
		mag_tmp.x = (s16)((s16)(mag_data_out[1]<<8)) | mag_data_out[0];
		mag_tmp.y = (s16)((s16)(mag_data_out[3]<<8)) | mag_data_out[2];
		mag_tmp.z = (s16)((s16)(mag_data_out[5]<<8)) | mag_data_out[4];

		mag_bias->x += (s32)(mag_tmp.x);
		mag_bias->y += (s32)(mag_tmp.y);
		mag_bias->z += (s32)(mag_tmp.z);
		for (j = 0; j < 3; j++) 
		{
			if(mag_temp[j] > mag_max[j]) 
			{
				mag_max[j] = mag_temp[j];
			}
			if(mag_temp[j] < mag_min[j])
			{ 
				mag_min[j] = mag_temp[j];
			}
		}
		usleep(55000);
	}
	mag_bias->x = (mag_max[0] + mag_min[0])/2;
	mag_bias->y = (mag_max[1] + mag_min[1])/2;
	mag_bias->z = (mag_max[2] + mag_min[2])/2;
	if(VERBOSE)
	{
		fprintf(stderr, esc1"Magnetometer bias output\t:\t%f \t%f \t%f\n"esc2, (float)mag_bias->x, (float)mag_bias->y, (float)mag_bias->z);
	}
	// Remove gravity from the z-axis accelerometer bias calculation


  	mraa_i2c_address(i2c, BNO055_I2C_BASE_ADDR);
  	mraa_i2c_write_byte_data(i2c, BNO055_MAG_OFFSET_X_LSB_ADDR, (s16)mag_bias->x & 0xFF);
  	mraa_i2c_write_byte_data(i2c, BNO055_MAG_OFFSET_X_MSB_ADDR, ((s16)mag_bias->x >> 8) & 0xFF);
  	mraa_i2c_write_byte_data(i2c, BNO055_MAG_OFFSET_Y_LSB_ADDR, (s16)mag_bias->y & 0xFF);
  	mraa_i2c_write_byte_data(i2c, BNO055_MAG_OFFSET_Y_MSB_ADDR, ((s16)mag_bias->y >> 8) & 0xFF);
  	mraa_i2c_write_byte_data(i2c, BNO055_MAG_OFFSET_Z_LSB_ADDR, (s16)mag_bias->z & 0xFF);
  	mraa_i2c_write_byte_data(i2c, BNO055_MAG_OFFSET_Z_MSB_ADDR, ((s16)mag_bias->z >> 8) & 0xFF);
  	mraa_i2c_stop(i2c);
}


void BNO055_getCompensatedMag(struct BNO055_mag_float_t* out)
{
	out->x = out->x-(float)(mag_bias.x)/BNO055_MAG_UT_DIVIDER;
	out->y = out->y-(float)(mag_bias.y)/BNO055_MAG_UT_DIVIDER;
	out->z = out->z-(float)(mag_bias.z)/BNO055_MAG_UT_DIVIDER;
}


void BNO055_getMag(struct BNO055_mag_float_t* out)
{
	u8 mag_data_out[MAG_REG_SIZE];

	s16 x;
	s16 y;
	s16 z;

	i2c = mraa_i2c_init_raw(I2CBUS_ADAPTER);
	mraa_i2c_address(i2c, BNO055_I2C_BASE_ADDR);
	mraa_i2c_read_bytes_data(i2c, BNO055_MAG_DATA_X_LSB_ADDR, mag_data_out, MAG_REG_SIZE);
	printf("BNO055 get magnetometer data...\n");
	if(VERBOSE)
	{
		if(i2c != NULL)
		{
			printf("%s\n", "i2c context pointer initialized");
			printf("%p\n", i2c);
		}
	}

	mraa_i2c_stop(i2c);

	x = (s16)(mag_data_out[1]<<8) | mag_data_out[0];
	y = (s16)(mag_data_out[3]<<8) | mag_data_out[2];
	z = (s16)(mag_data_out[5]<<8) | mag_data_out[4];

	out->x = (float)(x)/BNO055_MAG_UT_DIVIDER;
	out->y = (float)(y)/BNO055_MAG_UT_DIVIDER;
	out->z = (float)(z)/BNO055_MAG_UT_DIVIDER;
}

void BNO055_gyro_calib(struct BNO055_gyro_bias_t* gyro_bias)
{
	u8 mode;
	u8 gyro_data_out[GYRO_REG_SIZE];
	printf("Gyro Calibration: Put device on a level surface and keep motionless! Wait......\n");

	u16 sample_count = 256, i;
	struct BNO055_gyro_t gyro_tmp;

	i2c = mraa_i2c_init_raw(I2CBUS_ADAPTER);
	mraa_i2c_address(i2c, BNO055_I2C_BASE_ADDR);
	mode = mraa_i2c_read_byte_data(i2c, BNO055_OPR_MODE_ADDR);
	if(!(mode == BNO055_NDOF_FMC_OFF_MODE || mode == BNO055_NDOF_OPR_MODE))
	{
		printf("False mode\n");
	}
	gyro_bias->x = 0;
	gyro_bias->y = 0;
	gyro_bias->z = 0;

	for(i = 0; i < sample_count; i++)
	{
		mraa_i2c_read_bytes_data(i2c, BNO055_GYRO_DATA_X_LSB_ADDR, gyro_data_out, ACCEL_REG_SIZE);	
		gyro_tmp.x = (s16)((s16)(gyro_data_out[1]<<8)) | gyro_data_out[0];
		gyro_tmp.y = (s16)((s16)(gyro_data_out[3]<<8)) | gyro_data_out[2];
		gyro_tmp.z = (s16)((s16)(gyro_data_out[5]<<8)) | gyro_data_out[4];

		gyro_bias->x += (s32)(gyro_tmp.x);
		gyro_bias->y += (s32)(gyro_tmp.y);
		gyro_bias->z += (s32)(gyro_tmp.z);

		usleep(35000);
	}
	gyro_bias->x /= ((s32)sample_count);
	gyro_bias->y /= ((s32)sample_count);
	gyro_bias->z /= ((s32)sample_count);
	if(VERBOSE)
	{
		fprintf(stderr, esc1"Gyroscope bias output\t:\t%f \t%f \t%f\n"esc2, (float)gyro_bias->x, (float)gyro_bias->y, (float)gyro_bias->z);
	}
  	mraa_i2c_address(i2c, BNO055_I2C_BASE_ADDR);
  	mraa_i2c_write_byte_data(i2c, BNO055_GYRO_OFFSET_X_LSB_ADDR, (s16)gyro_bias->x & 0xFF);
  	mraa_i2c_write_byte_data(i2c, BNO055_GYRO_OFFSET_X_MSB_ADDR, ((s16)gyro_bias->x >> 8) & 0xFF);
  	mraa_i2c_write_byte_data(i2c, BNO055_GYRO_OFFSET_Y_LSB_ADDR, (s16)gyro_bias->y & 0xFF);
  	mraa_i2c_write_byte_data(i2c, BNO055_GYRO_OFFSET_Y_MSB_ADDR, ((s16)gyro_bias->y >> 8) & 0xFF);
  	mraa_i2c_write_byte_data(i2c, BNO055_GYRO_OFFSET_Z_LSB_ADDR, (s16)gyro_bias->z & 0xFF);
  	mraa_i2c_write_byte_data(i2c, BNO055_GYRO_OFFSET_Z_MSB_ADDR, ((s16)gyro_bias->z >> 8) & 0xFF);
  	mraa_i2c_stop(i2c);
}


void BNO055_getCompensatedGyro(struct BNO055_gyro_float_t* out)
{
	out->x = out->x-(float)(gyro_bias.x)/BNO055_GYRO_DPS_DIVIDER;
	out->y = out->y-(float)(gyro_bias.y)/BNO055_GYRO_DPS_DIVIDER;
	out->z = out->z-(float)(gyro_bias.z)/BNO055_GYRO_DPS_DIVIDER;
}


void BNO055_getGyro(struct BNO055_gyro_float_t* out)
{
	u8 gyro_data_out[GYRO_REG_SIZE];

	s16 x;
	s16 y;
	s16 z;

	i2c = mraa_i2c_init_raw(I2CBUS_ADAPTER);
	mraa_i2c_address(i2c, BNO055_I2C_BASE_ADDR);
	mraa_i2c_read_bytes_data(i2c, BNO055_GYRO_DATA_X_LSB_ADDR, gyro_data_out, GYRO_REG_SIZE);
	printf("BNO055 get gyroscope data...\n");
	if(VERBOSE)
	{		
		if(i2c != NULL)
		{
			printf("%s\n", "i2c context pointer initialized");
			printf("%p\n", i2c);
		}
	}
	
	mraa_i2c_stop(i2c);

	x = (s16)(gyro_data_out[1]<<8) | gyro_data_out[0];
	y = (s16)(gyro_data_out[3]<<8) | gyro_data_out[2];
	z = (s16)(gyro_data_out[5]<<8) | gyro_data_out[4];

	out->x = (float)(x)/BNO055_GYRO_DPS_DIVIDER;
	out->y = (float)(y)/BNO055_GYRO_DPS_DIVIDER;
	out->z = (float)(z)/BNO055_GYRO_DPS_DIVIDER;
}

void BNO055_getQuaternion(struct BNO055_quaternion_float_t* out)
{
	u8 quater_data_out[QUATERNION_REG_SIZE];

	s16 w;
	s16 x;
	s16 y;
	s16 z;

	i2c = mraa_i2c_init_raw(I2CBUS_ADAPTER);
	mraa_i2c_address(i2c, BNO055_I2C_BASE_ADDR);
	mraa_i2c_read_bytes_data(i2c, BNO055_QUATERNION_DATA_W_LSB_ADDR, quater_data_out, QUATERNION_REG_SIZE);
	printf("BNO055 get quaternion data...\n");
	
	if(VERBOSE)
	{
		if(i2c != NULL)
		{
			printf("%s\n", "i2c context pointer initialized");
			printf("%p\n", i2c);
		}
	}

	mraa_i2c_stop(i2c);

	w = (s16)(quater_data_out[1]<<8) | quater_data_out[0];
	x = (s16)(quater_data_out[3]<<8) | quater_data_out[2];
	y = (s16)(quater_data_out[5]<<8) | quater_data_out[4];
	z = (s16)(quater_data_out[7]<<8) | quater_data_out[6];

	out->w = (float)(w);
	out->x = (float)(x);
	out->y = (float)(y);
	out->z = (float)(z);
}

void BNO055_getEuler(struct BNO055_euler_float_t* out)
{
	u8 euler_data_out[GYRO_REG_SIZE];

	s16 h;
	s16 r;
	s16 p;

	i2c = mraa_i2c_init_raw(I2CBUS_ADAPTER);
	mraa_i2c_address(i2c, BNO055_I2C_BASE_ADDR);
	mraa_i2c_read_bytes_data(i2c, BNO055_EULER_H_LSB_ADDR, euler_data_out, EULER_REG_SIZE);
	if(VERBOSE)
	{		
		if(i2c != NULL)
		{
			printf("%s\n", "i2c context pointer initialized");
			printf("%p\n", i2c);
		}
	}
	
	mraa_i2c_stop(i2c);

	h = (s16)(euler_data_out[1]<<8) | euler_data_out[0];
	r = (s16)(euler_data_out[3]<<8) | euler_data_out[2];
	p = (s16)(euler_data_out[5]<<8) | euler_data_out[4];

	out->h = (float)(h)/BNO055_EULER_DIVIDER;
	out->r = (float)(r)/BNO055_EULER_DIVIDER;
	out->p = (float)(p)/BNO055_EULER_DIVIDER;
}