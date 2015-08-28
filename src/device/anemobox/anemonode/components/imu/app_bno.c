/**
 * \file app_bno.c
 * \brief Test program for the Inertial station BNO055
 * \author fom
 * \version 1.3
 * \date 7 mai 2015
 *
 * Test program for the Inertial station BNO055
 *
 */
#include "bno055.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>

int i;
char* platform;
u8 st;
u8 status;

int main(int argc, char** argv)
{
	res = mraa_init();
	platform = mraa_get_platform_name();

	if(VERBOSE)
	{
		printf("------------------------\n\n");
		printf("You are currently running this program on : %s\n\n", platform);
		if(res != MRAA_SUCCESS)
		{
			if(res == 12)
			{
				printf("Main started & mraa lib was already correctly initialized : %d\n", res);
			}
			else
			{
				exit(-1);
			}
		}
		printf("------------------------\n\n");
	}

	i2c = mraa_i2c_init_raw(I2CBUS_ADAPTER);

	mraa_i2c_address(i2c, BNO055_I2C_BASE_ADDR);
	status = mraa_i2c_read_byte_data(i2c, 0);

	setMode(BNO055_NDOF_OPR_MODE);

	while(1)
	{
		
		BNO055_getEuler(&bno055_euler_out);

		fprintf(stdout,"{\"h\":%f,\"r\":%f,\"p\":%f}\n", bno055_euler_out.h, bno055_euler_out.r, bno055_euler_out.p);

		SOME_TIME;
	}


	return 0;
}
