'use strict';

var path = require('path');

var rootPath = path.normalize(__dirname + '/../../..');

module.exports = {
  root: rootPath,
  mongo: {
    options: {
      db: {
        safe: true
      }
    }
  }
};
