'use strict';

angular.module('www2App')
  .filter('geopos', function () {
    var deg_to_dms = function (deg) {
      var d = Math.floor (deg);
      var minfloat = (deg-d)*60;
      var m = Math.floor(minfloat);
      var secfloat = (minfloat-m)*60;
      var s = secfloat;
      // After rounding, the seconds might become 60. These two
      // if-tests are not necessary if no rounding is done.
      if (m==60) {
        d++;
        m=0;
      }
      return ("" + d + "°" + m + "′" + s.toFixed(3) + "″");
    };

    return function (pos) {
      var latLon = Utils.worldToLatLon(pos);
      if (!latLon) {
        return 'n/a';
      }
      return deg_to_dms(Math.abs(latLon.lat)) + (latLon.lat >= 0 ? 'N' : 'S')
      + ',' + deg_to_dms(Math.abs(latLon.lon)) + (latLon.lon >= 0 ? 'E' : 'W');
    };
  });
