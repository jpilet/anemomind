'use strict';

angular.module('www2App')
  .filter('speed', function () {
    return function (speed) {
      if (speed == undefined) {
        return "n/a";
      }
      return speed.toFixed(1) +" kn";
    };
  });
