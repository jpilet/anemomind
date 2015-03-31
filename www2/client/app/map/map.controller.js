'use strict';

angular.module('www2App')
  .controller('MapCtrl', function ($scope, $stateParams, userDB, $http) {
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
});
