
var i2cModule = require('./i2c');
var i2c = i2cModule.i2c;
var anemonode = require('../build/Release/anemonode');
var fs = require('fs');
var Q = require('q');

var calibFile = "/home/anemobox/imu.cal";

var BNO055_I2C_BASE_ADDR = 0x29;
var BNO055_WHOAMI = 0xA0;
var BNO055_SYS_TRIGGER = 0x3F;
var BNO055_SYS_RST = (1<<5);
var BNO055_CALIB_STAT = 0x35;
var BNO055_ST_RESULT_ADDR = 0x36;
var BNO055_OPR_MODE_ADDR = 0x3D;
var BNO055_OPR_MODE_CONFIG_MODE = 0x00;
var BNO055_ACCONLY_OPR_MODE = 0x01;
var BNO055_NDOF_OPR_MODE = 0x0C;
var BNO055_NDOF_FMC_OFF_MODE = 0x0B;
var SYS_STATUS_IDLE = 0x00;
var SYS_STATUS_FUSION = 0x05;
var BNO055_PWR_MODE_ADDR = 0x3E;
var BNO055_PWR_MODE_NORMAL = 0x00;
var BNO055_PWR_MODE_LOW_PWR = 0x01;
var BNO055_SYS_ERR = 0x3A;
var BNO055_SYS_STATUS = 0x39;
var BNO055_UNIT_SEL_ADDR = 0x3B;
var BNO055_ACC_MPSS_DIVIDER = 100.0;
var BNO055_ACC_MG_DIVIDER = 1.0;
var G_SCALE = 1000.0;
var BNO055_MAG_UT_DIVIDER = 16.0;
var BNO055_GYRO_DPS_DIVIDER = 16.0;
var BNO055_GYRO_RPS_DIVIDER = 900.0;
var BNO055_EULER_DIVIDER = 16.0;

/* Page id register definition*/
var BNO055_PAGE_ID_ADDR = 0X07;

/* PAGE0 REGISTER DEFINITION START*/
var BNO055_CHIP_ID_ADDR = 0x00;
var BNO055_ACCEL_REV_ID_ADDR = 0x01;
var BNO055_MAG_REV_ID_ADDR = 0x02;
var BNO055_GYRO_REV_ID_ADDR = 0x03;
var BNO055_SW_REV_ID_LSB_ADDR = 0x04;
var BNO055_SW_REV_ID_MSB_ADDR = 0x05;
var BNO055_BL_REV_ID_ADDR = 0X06;

/* Accel data register*/
var BNO055_ACCEL_DATA_X_LSB_ADDR = 0X08;
var BNO055_ACCEL_DATA_X_MSB_ADDR = 0X09;
var BNO055_ACCEL_DATA_Y_LSB_ADDR = 0X0A;
var BNO055_ACCEL_DATA_Y_MSB_ADDR = 0X0B;
var BNO055_ACCEL_DATA_Z_LSB_ADDR = 0X0C;
var BNO055_ACCEL_DATA_Z_MSB_ADDR = 0X0D;

/*Mag data register*/
var BNO055_MAG_DATA_X_LSB_ADDR = 0X0E;
var BNO055_MAG_DATA_X_MSB_ADDR = 0X0F;
var BNO055_MAG_DATA_Y_LSB_ADDR = 0X10;
var BNO055_MAG_DATA_Y_MSB_ADDR = 0X11;
var BNO055_MAG_DATA_Z_LSB_ADDR = 0X12;
var BNO055_MAG_DATA_Z_MSB_ADDR = 0X13;

/*Gyro data registers*/
var BNO055_GYRO_DATA_X_LSB_ADDR = 0X14;
var BNO055_GYRO_DATA_X_MSB_ADDR = 0X15;
var BNO055_GYRO_DATA_Y_LSB_ADDR = 0X16;
var BNO055_GYRO_DATA_Y_MSB_ADDR = 0X17;
var BNO055_GYRO_DATA_Z_LSB_ADDR = 0X18;
var BNO055_GYRO_DATA_Z_MSB_ADDR = 0X19;

/*Quaternion data registers*/
var BNO055_QUATERNION_DATA_W_LSB_ADDR = 0X20;
var BNO055_QUATERNION_DATA_W_MSB_ADDR = 0X21;
var BNO055_QUATERNION_DATA_X_LSB_ADDR = 0X22;
var BNO055_QUATERNION_DATA_X_MSB_ADDR = 0X23;
var BNO055_QUATERNION_DATA_Y_LSB_ADDR = 0X24;
var BNO055_QUATERNION_DATA_Y_MSB_ADDR = 0X25;
var BNO055_QUATERNION_DATA_Z_LSB_ADDR = 0X26;
var BNO055_QUATERNION_DATA_Z_MSB_ADDR = 0X27;

/* Accel offset register*/
var BNO055_ACCEL_OFFSET_X_LSB_ADDR = 0X55;
var BNO055_ACCEL_OFFSET_X_MSB_ADDR = 0X56;
var BNO055_ACCEL_OFFSET_Y_LSB_ADDR = 0X57;
var BNO055_ACCEL_OFFSET_Y_MSB_ADDR = 0X58;
var BNO055_ACCEL_OFFSET_Z_LSB_ADDR = 0X59;
var BNO055_ACCEL_OFFSET_Z_MSB_ADDR = 0X5A;

/*Mag offset register*/
var BNO055_MAG_OFFSET_X_LSB_ADDR = 0X5B;
var BNO055_MAG_OFFSET_X_MSB_ADDR = 0X5C;
var BNO055_MAG_OFFSET_Y_LSB_ADDR = 0X5D;
var BNO055_MAG_OFFSET_Y_MSB_ADDR = 0X5E;
var BNO055_MAG_OFFSET_Z_LSB_ADDR = 0X5F;
var BNO055_MAG_OFFSET_Z_MSB_ADDR = 0X60;

/*Gyro offset registers*/
var BNO055_GYRO_OFFSET_X_LSB_ADDR = 0X61;
var BNO055_GYRO_OFFSET_X_MSB_ADDR = 0X62;
var BNO055_GYRO_OFFSET_Y_LSB_ADDR = 0X63;
var BNO055_GYRO_OFFSET_Y_MSB_ADDR = 0X64;
var BNO055_GYRO_OFFSET_Z_LSB_ADDR = 0X65;
var BNO055_GYRO_OFFSET_Z_MSB_ADDR = 0X66;

var BNO055_MAG_RADIUS_MSB = 0x6A;

var BNO055_EULER_H_LSB_ADDR = 0X1A;
var BNO055_EULER_H_MSB_ADDR = 0X1B;
var BNO055_EULER_R_LSB_ADDR = 0X1C;
var BNO055_EULER_R_MSB_ADDR = 0X1D;
var BNO055_EULER_P_LSB_ADDR = 0X1E;
var BNO055_EULER_P_MSB_ADDR = 0X1F;


function readCalib() {
  var deferred = Q.defer();
  fs.readFile(calibFile, function (err, data) {
    if (data) {
      var calibrationData = JSON.parse(data);
      deferred.resolve(calibrationData);
    } else {
      deferred.reject(new Error(err));
    }
  });
  return deferred.promise;
}

var calibrationSaved = false;

function init(done) {
  i2c.address(BNO055_I2C_BASE_ADDR);
  if (i2c.readReg(BNO055_CHIP_ID_ADDR) != BNO055_WHOAMI) {
    console.log("Can't find BNO055 chip.");
    return;
  }
  i2c.address(BNO055_I2C_BASE_ADDR);
  i2c.writeReg(BNO055_OPR_MODE_ADDR, BNO055_OPR_MODE_CONFIG_MODE);
  Q.delay(30)
  .then(readCalib)
  .then(function(data) {
      i2c.address(BNO055_I2C_BASE_ADDR);
      if (data && data.length == 1 + BNO055_MAG_RADIUS_MSB - BNO055_ACCEL_OFFSET_X_LSB_ADDR) {
        for (var reg =  BNO055_ACCEL_OFFSET_X_LSB_ADDR;
             reg <= BNO055_MAG_RADIUS_MSB; ++reg) {
          var i = reg - BNO055_ACCEL_OFFSET_X_LSB_ADDR;
          i2c.writeReg(reg, data[i]); 
        }
        calibrationSaved = true;
      }
    })
  .then(Q.delay(100))
  .finally(function() {
    i2c.address(BNO055_I2C_BASE_ADDR);
    i2c.writeReg(BNO055_OPR_MODE_ADDR, BNO055_OPR_MODE_CONFIG_MODE);
    i2c.writeReg(BNO055_OPR_MODE_ADDR, BNO055_NDOF_OPR_MODE);
    setTimeout(done, 50);
  });
}

function getAngles() {
  try {
    i2c.address(BNO055_I2C_BASE_ADDR);
    var data = i2c.readBytesReg(BNO055_EULER_H_LSB_ADDR, 6);
    var h = data.readInt16LE(0);
    var r = data.readInt16LE(2);
    var p = data.readInt16LE(4);
    var divider = 1.0 / 16.0;
    return {
      heading: h * divider,
      roll: r * divider,
      pitch: p * divider
    };
  } catch(err) {
    console.warn(err);
    i2cModule.reset();
    return undefined;
  }
}

function isCalibrated() {
  i2c.address(BNO055_I2C_BASE_ADDR);
  return (i2c.readReg(BNO055_CALIB_STAT) & 0x3) == 0x3;
}

function saveCalib(done) {
  if (calibrationSaved) {
    //console.log('already calibrated');
    return;
  }

  if (!isCalibrated()) {
    //console.log('not calibrated.');
    return;
  }
  i2c.address(BNO055_I2C_BASE_ADDR);
  i2c.writeReg(BNO055_OPR_MODE_ADDR, BNO055_OPR_MODE_CONFIG_MODE);

  Q.delay(100).then(function() {
    i2c.address(BNO055_I2C_BASE_ADDR);
    var data = [];
    for (var reg =  BNO055_ACCEL_OFFSET_X_LSB_ADDR;
         reg <= BNO055_MAG_RADIUS_MSB; ++reg) {
          data.push(i2c.readReg(reg));
    }
    var json = JSON.stringify(data);

    fs.writeFile(calibFile, json, function(err) {
      if(err) {
        return console.log(err);
      }
      calibrationSaved = true;
      console.log('IMU calibration SAVED.');
    }); 
    i2c.writeReg(BNO055_OPR_MODE_ADDR, BNO055_NDOF_OPR_MODE);

    setTimeout(done, 20);
  });
}

function readImu() {
  // these are box angles.
  // TODO: convert them in a boat coordinate system.
  var angles = getAngles();
  if (angles) {
    anemonode.dispatcher.values.orient.setValue("IMU", angles);
  }
}

module.exports.init = init;
module.exports.getAngles = getAngles;
module.exports.isCalibrated = isCalibrated;
module.exports.saveCalib = saveCalib;
module.exports.readImu = readImu;
