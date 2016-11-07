'use strict';

angular.module('www2App')
  .config(function ($stateProvider) {
    $stateProvider
      .state('challenges', {
        url: '/challenges',
        templateUrl: 'app/challenges/challenges.html',
        controller: 'ChallengesCtrl'
      });
  });
