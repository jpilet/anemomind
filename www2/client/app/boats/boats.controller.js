'use strict';

angular.module('www2App')
  .controller('BoatsCtrl', function ($scope, $http) {
    $http.get('/api/boats')
    .success(function(data, status, headers, config) {
       $scope.boats = data;
     });
  });
