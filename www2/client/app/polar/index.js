'use strict';

angular.module('www2App')
  .config(function ($stateProvider) {
    $stateProvider
      .state('polar', {
        url: '/polar/:boatId/:name',
        templateUrl: 'app/polar/polar.html',
        controller: 'PolarCtrl'
      });
  });

