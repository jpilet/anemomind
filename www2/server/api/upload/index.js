'use strict';

var express = require('express');
var controller = require('./upload.controller');
var config = require('../../config/environment');
var router = express.Router();
var multer  = require('multer');


var m = multer({ dest: config.uploadDir,
  rename: function (fieldname, filename) {
    return filename+Date.now();
  },
  onFileUploadStart: function (file) {
    console.log(file.originalname + ' is starting ...')
  },
  onFileUploadComplete: function (file) {
    console.log(file.fieldname + ' uploaded to  ' + file.path)
  }
});

router.post('/', m, controller.upload);

module.exports = router;