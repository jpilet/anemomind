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
      $scope.currentTime = new Date($scope.currentTime.getTime()+20 * 1000);
      if ($scope.currentTime >= $scope.plotData[$scope.plotData.length - 1]['time']) {
        $scope.currentTime = new Date($scope.plotData[0]['time']);
      }
    }

    $scope.$watch('mapLocation', setLocation);
    $scope.$watch('selectedCurve', setLocation);
});
