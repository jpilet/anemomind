'use strict';

function perfAtPoint(d) {
    var field = 'devicePerf';
    if (field in d) {
      return d[field];
    } else if ('deviceVmg' in d
               && 'deviceTargetVmg' in d) {
      return Math.round(Math.abs(100 * d.deviceVmg / d.deviceTargetVmg));
    }
    return 0;
}

function vmgAtPoint(p) {
  if ('deviceVmg' in p) {
    return Math.abs(p.deviceVmg);
  }
  return undefined;
}

angular.module('www2App')
  .controller('MapCtrl', function ($scope, $stateParams, userDB, $timeout,
                                   $http, $interval, $state, $location) {

    $scope.boat = { _id: $stateParams.boatId, name: 'loading' };

    var setLocationTimeout;
    function setLocation() {
      function delayed() {
        setLocationTimeout = undefined;
        var search = '';
        if ($scope.mapLocation) {
          var l = $scope.mapLocation;
          search += 'l=' + l.x +',' + l.y + ',' + l.scale;
        }
        if ($scope.selectedCurve) {
          search += '&c=' + $scope.selectedCurve;
        }
        $location.search(search).replace();
      }

      if (setLocationTimeout) {
        $timeout.cancel(setLocationTimeout);
      }
      setLocationTimeout = $timeout(delayed, 1000);
    }

    if ($stateParams.c) {
      $scope.selectedCurve = $stateParams.c;
    }

    if ($stateParams.l) {
      var entries = $stateParams.l.split(',');
      $scope.mapLocation = {
        x: parseFloat(entries[0]),
        y: parseFloat(entries[1]),
        scale: parseFloat(entries[2])
      };
    }

    $http.get('/api/boats/' + $stateParams.boatId)
    .success(function(data, status, headers, config) {
      $scope.boat = data;

    });

    $scope.plotField = 'devicePerf';

    $scope.plotFieldLabels = {
      'gpsSpeed' : 'Speed over ground (GPS)',
      'devicePerf' : 'VMG performance',
      'aws' : 'Apparent wind speed',
      'deviceTws' : 'True wind speed (Anemomind)',
      'externalTws' : 'True wind speed (onboard instruments)',
      'watSpeed': 'Water speed',
      'deviceVmg': 'VMG',
      'deviceTargetVmg': 'Target VMG'
      // those can't be displayed because they are angles:
      // awa deviceTwdir externalTwa gpsBearing magHdg
    };

    $scope.isPlaying = false;
    var animationTimer;

    $scope.togglePlayPause = function() {
      $scope.isPlaying = !$scope.isPlaying;
    }

    var lastPositionUpdate = new Date();

    $scope.$watch('isPlaying', function(newVal, oldVal) {
      if (newVal != oldVal) {
        if (newVal) {
          lastPositionUpdate = new Date();
          animationTimer = $interval(updatePosition, 100);
        } else {
          $interval.cancel(animationTimer);
        }
      }
    });

    function updatePosition() {
      var now = new Date();
      if(!$scope.currentTime){
        $scope.currentTime = new Date($scope.plotData[0]['time']);
      } else {
        var delta = (now.getTime() - lastPositionUpdate.getTime());
        delta *= $scope.replaySpeed;
        $scope.currentTime = new Date($scope.currentTime.getTime() + delta);
        if ($scope.currentTime >= $scope.plotData[$scope.plotData.length - 1]['time']) {
          $scope.currentTime = new Date($scope.plotData[0]['time']);
        }
      }
      lastPositionUpdate = now;
    }

    $scope.$watch('mapLocation', setLocation);
    $scope.$watch('selectedCurve', setLocation);

    var pointAtTime = function(time) {
      if (!time || !$scope.plotData || $scope.plotData.length < 2) {
        return {};
      }

      // TODO: move this function in a library.
      var binarySearch = function(list, item) {
        var min = 0;
        var max = list.length - 1;
        var guess;

        while ((max - min) > 1) {
            guess = Math.floor((min + max) / 2);

            if (list[guess].time < item) {
                min = guess;
            }
            else {
                max = guess;
            }
        }

        return [min, max];
      };

      var bounds = binarySearch($scope.plotData, time);
      var delta = [
        Math.abs($scope.plotData[bounds[0]].time - time),
        Math.abs($scope.plotData[bounds[1]].time - time)];
      var s = (delta[0] < delta[1] ? 0 : 1);
      return $scope.plotData[bounds[s]];
    };

    function getPointValue(keys) {
      if ($scope.currentPoint == undefined) {
        return undefined;
      }
      for (var i in keys) {
        var key = keys[i];
        if (key in $scope.currentPoint) {
          return $scope.currentPoint[key];
        }
      }
      return undefined;
    }

    function twdir() {
      var twa = getPointValue(['twa', 'externalTwa']);
      if (twa == undefined) {
        return twa;
      }
      var bearing = getPointValue(['gpsBearing']);
      return bearing + twa;
    }

    $scope.currentPoint = pointAtTime($scope.currentTime);
    $scope.$watch('currentTime', function(time) {
      $scope.currentPoint = pointAtTime(time);

      $scope.vmgPerf = perfAtPoint($scope.currentPoint);
      $scope.twa = getPointValue(['twa', 'externalTwa']);
      $scope.tws =  getPointValue(['twa', 'externalTws']);
      $scope.gpsSpeed = getPointValue(['gpsSpeed']);
      $scope.twdir = twdir();
      $scope.gpsBearing = getPointValue(['gpsBearing']);
      $scope.deviceVmg = getPointValue(['deviceVmg']);
      if ($scope.deviceVmg) {
        $scope.deviceVmg = Math.abs($scope.deviceVmg);
      }
      $scope.deviceTargetVmg = getPointValue(['deviceTargetVmg']);
    });

    $scope.replaySpeed = 8;
    $scope.slower = function() { $scope.replaySpeed /= 2; }
    $scope.faster = function() { $scope.replaySpeed *= 2; }

});
