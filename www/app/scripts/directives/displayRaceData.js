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
    scope.current = {};
    scope.$watch('currentPos', function(currentPos) {
      if(scope.data) {
        var elapsedSeconds = (scope.data[currentPos].timeMs - scope.data[0].timeMs) / 1000;
        var hours = parseInt(elapsedSeconds / 3600) % 24;
        var minutes = parseInt(elapsedSeconds / 60) % 60;
        var seconds = parseInt(elapsedSeconds % 60, 10);
        scope.current.elapsed = {
          hours: hours < 10 ? '0' + hours : hours,
          minutes: minutes < 10 ? '0' + minutes : minutes,
          seconds: seconds < 10 ? '0' + seconds : seconds
        };
        scope.current.awaRad = scope.data[currentPos].awaRad.toFixed(2);
        scope.current.awsMps = scope.data[currentPos].awsMps.toFixed(2);
        scope.current.twaRad = scope.data[currentPos].twaRad.toFixed(2);
        scope.current.twsMps = scope.data[currentPos].twsMps.toFixed(2);
        scope.current.watSpeedMps = scope.data[currentPos].watSpeedMps.toFixed(2);
        scope.current.gpsSpeedMps = scope.data[currentPos].gpsSpeedMps.toFixed(2);
        scope.current.magHdgRad = scope.data[currentPos].magHdgRad.toFixed(2);
        scope.current.gpsBearingRad = scope.data[currentPos].gpsBearingRad.toFixed(2);
      }
    });
  }
});