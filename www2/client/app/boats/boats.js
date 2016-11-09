'use strict';

angular.module('www2App')
  .config(function ($stateProvider) {
    $stateProvider
      .state('boat-list', {
        url: '/boats',
        templateUrl: 'app/boats/boats.html',
        controller: 'BoatsCtrl'
      })
      .state('boat-view', {
        url: '/boats/:boatId',
        templateUrl: 'app/boats/boat-display.html',
        controller: 'BoatsCtrl'
      })
      .state('boat-home', {
        url: '/',
        templateUrl: 'app/boats/boat-display.html',
        controller: 'BoatsCtrl'
      })

      .state('boat-detail', {
        url: '/boats/:boatId/edit',
        templateUrl: 'app/boats/boat-detail.html',
        controller: 'BoatDetailCtrl'
      });
  });
