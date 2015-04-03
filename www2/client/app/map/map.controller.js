'use strict';

angular.module('www2App')
  .controller('MapCtrl', function ($scope, $stateParams, userDB, $http, $interval) {
    $scope.boat = { name: 'loading' };

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
});