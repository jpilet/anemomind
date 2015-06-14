'use strict';

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
      'watSpeed': 'Water speed'
      // those can't be displayed because they are angles:
      // awa deviceTwdir externalTwa gpsBearing magHdg
    };

    $scope.isPlaying = false;
    var animationTimer;

    $scope.togglePlayPause = function() {
      $scope.isPlaying = !$scope.isPlaying;
    }

    $scope.$watch('isPlaying', function(newVal, oldVal) {
      if (newVal != oldVal) {
        if (newVal) {
          animationTimer = $interval(updatePosition, 100);
        } else {
          $interval.cancel(animationTimer);
        }
      }
    });

    function updatePosition() {
      if(!$scope.currentTime){
        $scope.currentTime = new Date($scope.plotData[0]['time']);
      }
      $scope.currentTime = new Date($scope.currentTime.getTime()+20 * 1000);
      if ($scope.currentTime >= $scope.plotData[$scope.plotData.length - 1]['time']) {
        $scope.currentTime = new Date($scope.plotData[0]['time']);
      }
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

    $scope.currentPoint = pointAtTime($scope.currentTime);
    $scope.$watch('currentTime', function(time) {
      $scope.currentPoint = pointAtTime(time);
    });
});
