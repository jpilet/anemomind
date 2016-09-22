'use strict';

angular.module('www2App')
  .config(function ($stateProvider) {
    $stateProvider
      .state('map', {
        url: '/map/:boatId?c&l&preview&queue',
        templateUrl: 'app/map/map.html',
        controller: 'MapCtrl',
        reloadOnSearch: false
      });
  });
