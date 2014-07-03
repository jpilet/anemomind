'use strict';

angular.module('anemomindApp.directives')
.directive('displayRaceData', function() {

  return {
    restrict: 'E',
    link: link,
    template: '<span class="label label-info">Elapsed time: {{current.elapsed.hours}}:{{current.elapsed.minutes}}:{{current.elapsed.seconds}}</span> ' +
              '<span class="label label-info">AWA: {{current.awaRad}}</span> ' +
              '<span class="label label-info">AWS: {{current.awsMps}}</span> ' +
              '<span class="label label-info">TWA: {{current.twaRad}}</span> ' +
              '<span class="label label-info">TWS: {{current.twsMps}}</span> ' +
              '<span class="label label-info">Water speed: {{current.watSpeedMps}}</span> ' +
              '<span class="label label-info">GPS Speed: {{current.gpsSpeedMps}}</span> ' +
              '<span class="label label-info">Mag. Heading: {{current.magHdgRad}}</span> ' +
              '<span class="label label-info">GPS Bearing: {{current.gpsBearingRad}}</span> '
  };

  function link(scope, element) {
  }
});