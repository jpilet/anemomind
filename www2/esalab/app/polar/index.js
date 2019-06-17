'use strict';

angular.module('www2App')
  .config(function ($stateProvider) {
    $stateProvider
      .state('polarIndex', {
        url: '/polar/:boatId',
        templateUrl: 'app/polar/polar_index.html',
        controller: 'PolarIndexCtrl'
      })
      .state('polar', {
        url: '/polar/:boatId/:urlName',
        templateUrl: 'app/polar/polar.html',
        controller: 'PolarCtrl'
      });
  });

