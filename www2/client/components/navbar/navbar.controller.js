'use strict';

angular.module('www2App')
  .controller('NavbarCtrl', function ($scope, $location, Auth, $http, socket) {
    $scope.menu = [{
      'title': 'Home',
      'link': '/'
    }];

    $scope.boats = [];

    if (Auth.isLoggedIn()) {
      $http.get('/api/boats')
        .success(function(data, status, headers, config) {
           $scope.boats = data;
           socket.syncUpdates('boat', $scope.boats);
        });
    }

    $scope.isCollapsed = true;
    $scope.isLoggedIn = Auth.isLoggedIn;
    $scope.isAdmin = Auth.isAdmin;
    $scope.getCurrentUser = Auth.getCurrentUser;

    $scope.logout = function() {
      Auth.logout();
      $location.path('/login');
    };

    $scope.isActive = function(route) {
      return route === $location.path();
    };
  });
