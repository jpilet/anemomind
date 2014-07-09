'use strict';

angular.module('anemomindApp.directives')
.directive('displayRaceData', function() {

  return {
    restrict: 'E',
    link: link,
    template: '<span class="label label-info">Time: {{formatTime(current.timeMs)}}</span> ' +
              '<span class="label label-info">AWA: {{formatAngle(current.awaRad)}}</span> ' +
              '<span class="label label-info">AWS: {{formatSpeed(current.awsMps)}}</span> ' +
              '<span class="label label-info">TWDir: {{formatDirection(current.twdirRad)}}</span> ' +
              '<span class="label label-info">eTWA: {{formatAngle(current.externalTwaRad)}}</span> ' +
              '<span class="label label-info">eTWS: {{formatSpeed(current.externalTwsMps)}}</span> ' +
              '<span class="label label-info">TWS: {{formatSpeed(current.twsMps)}}</span> ' +
              '<span class="label label-info">Water speed: {{formatSpeed(current.watSpeedMps)}}</span> ' +
              '<span class="label label-info">GPS Speed: {{formatSpeed(current.gpsSpeedMps)}}</span> ' +
              '<span class="label label-info">Mag. Heading: {{formatDirection(current.magHdgRad)}}</span> ' +
              '<span class="label label-info">GPS Bearing: {{formatDirection(current.gpsBearingRad)}}</span> '
  };

  function link(scope, element) {
  }
});
