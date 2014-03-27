'use strict';

angular.module('anemomindApp')
  .service('Myservice', function Myservice() {
    // AngularJS will instantiate a singleton by calling "new" on this function
    console.log('Myservice running...');
  });
