'use strict';

var mongoose = require('mongoose'),
    RaceData = mongoose.model('RaceData');

/**
 * Get list of races for specific user/boat
 */
exports.list = function (req, res) {
  var userId = req.session.passport.user;
  console.log(userId);

  RaceData.find({boatId: userId}, 'title boatId', function (err, raceList) {
    res.send(raceList);
  });
};

/**
 * Get race details
*/
exports.raceDetail = function(req, res) {
  var raceId = req.params.id;

  RaceData.findById(raceId, function (err, race) {
    if (err) return res.send(500);
    if (!race) return res.send(404);

    var formattedData = {
        origin: minAxis(race.items),
        coords: [],
        data: []
      };
    var ref = new GeoRef(formattedData.origin.x, formattedData.origin.y, 0);

    for (var i = 0; i < race.items.length; i++) {
      // take only 1 out of 10 coords
      if (i % 10 === 0) {
        var projected = ref.project(race.items[i]['latRad'], race.items[i]['lonRad']);
        var tmpCoords = {
          x_m: projected.x,
          y_m: projected.y,
        };
        var tmpData = race.items[i];
        formattedData.coords.push(tmpCoords);
        formattedData.data.push(tmpData);
      }
    }
    res.send(formattedData);
  });
};

function minAxis(array) {

  var minAxis = {x: Number.POSITIVE_INFINITY, y: Number.POSITIVE_INFINITY};
  for (var i=array.length-1; i>=0; i--) {
    minAxis.x = Math.min(minAxis.x, array[i]['latRad']);
    minAxis.y = Math.min(minAxis.y, array[i]['lonRad']);
  }
  return minAxis;
}

/*
 * Converts latitude and longitude to a local XY coordinate system.
 *
 * Usage example:
 * // Construct a single georef object from a representative
 * // coordinate.
 * var ref = new GeoRef(representativeLat, representativeLon, altitude);
 *
 * // Project a point.
 * var projected = ref.project(lat, lon);
 * // now projected contains x and y properties. These are distances
 * // in meters along the east/west axis to the reference point (x),
 * // and north/south for the y property.
 *
 */

function GeoRef(latRad, lonRad, altitude) {
  var a = 6378137; // semi-major axis of ellipsoid
  var f = 1.0/298.257223563; // flatening of ellipsoid
  var sinlat = Math.sin(latRad);
  var coslat = Math.cos(latRad);
  var sinlon = Math.sin(lonRad);
  var coslon = Math.cos(lonRad);
  var e2 =  f*(2-f); //  eccentricity^2
  var t3,t4,t5,t6,t8,t9,t11,t13,t16,t17,t18,t19,t23,t26,t31,t36,t37;

  /* mapple code:
      with(linalg);
      v := a/sqrt(1-e^2*sin(lat)^2);
      X := (v+altitude)*cos(lat)*cos(lon);
      Y := (v+altitude)*cos(lat)*sin(lon);
      Z := (v*(1-e^2)+h)*sin(lat);
      J := jacobian([X,Y,Z], [lon,lat]);
      llon := sqrt((J[1,1]^2 + J[2,1]^2 + J[3,1]^2));
      llat := sqrt((J[1,2]^2 + J[2,2]^2 + J[3,2]^2));
      r := vector([X,Y,Z,llon,llat]);
   */

  t3 = sinlat*sinlat;
  t4 = e2*t3;
  t5 = 1.0-t4;
  t6 = Math.sqrt(t5);
  t8 = a/t6;
  t9 = t8+altitude;
  t11 = t9*coslat;
  t13 = t11*sinlon;
  t16 = a/t6/t5;
  t17 = t16*e2;
  t18 = coslat*coslat;
  t19 = sinlat*t18;
  t23 = t9*sinlat;
  t26 = t11*coslon;
  t31 = 1.0-e2;
  t36 = t8*t31+altitude;
  t37 = t17*t19;

  this.reflon = lonRad;
  this.reflat = latRad;
  this.dlon = Math.sqrt(t13*t13 + t26*t26);

  var u = (t37*coslon-t23*coslon);
  var v = (t37*sinlon-t23*sinlon);
  var w = (t16*t31*t4*coslat+t36*coslat);
  this.dlat = Math.sqrt(u*u + v*v + w*w);
}

GeoRef.prototype.project = function(latRad, lonRad) {
  return {
    x: this.dlon * (lonRad - this.reflon),
    y: this.dlat * (latRad - this.reflat)
  };
};
