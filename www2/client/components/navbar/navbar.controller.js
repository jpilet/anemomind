'use strict';

angular.module('www2App')
  .controller('NavbarCtrl', function ($scope, $location, Auth, $http, socket, boatList) {
    $scope.menu = [{
      'title': 'Home',
      'link': '/'
    }];

    boatList.boats().then(function(boats) {
      $scope.boats = boats;
    });

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
