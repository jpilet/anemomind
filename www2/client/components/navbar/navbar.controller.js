'use strict';

angular.module('www2App')
  .controller('NavbarCtrl', function ($scope, $location, Auth, $http, socket, boatList) {
    $scope.showLinks = false;
    $scope.menu = [{
      'title': 'Home',
      'link': '/'
    }];

    $scope.boats = boatList.boats();
    $scope.$on('boatList:updated', function(event,data) {
      $scope.boats = data;
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

    if ($location.search().showLinks) {
      $scope.showLinks = true;
    }
  });
