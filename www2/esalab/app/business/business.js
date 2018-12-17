'use strict';

angular.module('www2App')
  .config(function ($stateProvider) {
    $stateProvider
      .state('business', {
        url: '/business',
        templateUrl: 'app/business/business.html',
        controller: 'BusinessCtrl'
      });
  });
