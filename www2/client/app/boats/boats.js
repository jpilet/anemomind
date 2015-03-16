'use strict';

angular.module('www2App')
  .config(function ($stateProvider) {
    $stateProvider
      .state('boats', {
        url: '/boats',
        templateUrl: 'app/boats/boats.html',
        controller: 'BoatsCtrl'
      })
      .state('boat-detail', {
        url: '/boats/:boatId',
        templateUrl: 'app/boats/boat-detail.html',
        controller: 'BoatDetailCtrl'
      });
  });
