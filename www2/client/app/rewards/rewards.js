'use strict';

angular.module('www2App')
  .config(function ($stateProvider) {
    $stateProvider
      .state('rewards', {
        url: '/rewards',
        templateUrl: 'app/rewards/rewards.html',
        controller: 'RewardsCtrl'
      });
  });
