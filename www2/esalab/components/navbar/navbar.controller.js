'use strict';

angular.module('www2App')
  .controller('NavbarCtrl', function ($scope, $location, Auth, $http, socket, boatList) {
    $scope.showLinks = false;
    $scope.menu = [{
      'title': 'Home',
      'link': '/'
    }, {
      'title': 'About',
      'link': '/about'
    }, {
      'title': 'Contact',
      'link': '/contact'
    }
    ];

    boatList.boats().then(function(boats) {
      var displayNavbarBoats=11;
      $scope.boats = boats;
      $scope.limit = Math.min(displayNavbarBoats,boats.length);
      $scope.offset= Math.max(boats.length-displayNavbarBoats, 0);
      $scope.truncated = boats.length > displayNavbarBoats;
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
