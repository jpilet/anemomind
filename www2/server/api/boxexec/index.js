'use strict';

var express = require('express');
var controller = require('./boxexec.controller');

var router = express.Router();

/*

  Maybe we want to activate these
  routes in case we want to have a
  web interface for looking at script results.
  
  router.get('/', controller.index);
  router.get('/:id', controller.show);
  router.post('/', controller.create);
  router.put('/:id', controller.update);
  router.patch('/:id', controller.update);
  router.delete('/:id', controller.destroy);
  
*/
module.exports = router;
