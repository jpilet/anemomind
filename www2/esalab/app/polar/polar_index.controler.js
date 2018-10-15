'use strict';

angular.module('www2App')
  .controller('PolarIndexCtrl', function ($scope, $stateParams, $http, Auth, boatList) {

  $scope.boatId=$stateParams.boatId;
  $scope.perfstatIndex = [];

  boatList.boat($stateParams.boatId)
    .then(function (boat) { $scope.boat = boat; });

  $http.get('/api/perfstat/' + $stateParams.boatId)
    .success(function(data, status, headers, config) {
      $scope.perfstatIndex = data;
    });

});

