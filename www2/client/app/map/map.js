'use strict';

angular.module('www2App')
  .config(function ($stateProvider) {
    $stateProvider
      .state('map', {
        url: '/map/:boatId',
        templateUrl: 'app/map/map.html',
        controller: 'MapCtrl'
      });
  });
