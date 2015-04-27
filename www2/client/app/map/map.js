'use strict';

angular.module('www2App')
  .config(function ($stateProvider) {
    $stateProvider
      .state('map', {
        url: '/map/:boatId?c&l',
        templateUrl: 'app/map/map.html',
        controller: 'MapCtrl',
        reloadOnSearch: false
      });
  });
