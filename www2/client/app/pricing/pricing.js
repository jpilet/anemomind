'use strict';

angular.module('www2App')
  .config(function ($stateProvider) {
    $stateProvider
      .state('pricing', {
        url: '/pricing',
        templateUrl: 'app/pricing/pricing.html',
        controller: 'PricingCtrl'
      })
  });
