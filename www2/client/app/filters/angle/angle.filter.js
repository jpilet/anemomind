'use strict';

angular.module('www2App')
  .filter('angle', function () {
    return function (angle, max) {
      if (angle == undefined) {
        return "n/a";
      } else {
        if (max == undefined) {
          max = 360;
        }
        var min = max - 360;

        while (angle < min) {
          angle += 360;
        }
        while (angle > max) {
          angle -= 360;
        }
        return "" + Math.round(angle);
      }
    };
  });
