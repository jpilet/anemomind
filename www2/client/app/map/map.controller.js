'use strict';

angular.module('www2App')
  .controller('MapCtrl', function ($scope, $stateParams, userDB, $http) {
    $scope.boat = { name: 'loading' };

    $http.get('/api/boats/' + $stateParams.boatId)
    .success(function(data, status, headers, config) {
      $scope.boat = data;
    });
});
