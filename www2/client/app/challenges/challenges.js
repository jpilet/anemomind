'use strict';

angular.module('www2App')
  .config(function ($stateProvider) {
    $stateProvider
      .state('challenges', {
        url: '/challenges',
        templateUrl: 'app/challenges/challenges.html',
        controller: 'ChallengesCtrl'
      })
      .state('challenge-list', {
        url: '/challenge/list',
        templateUrl: 'app/challenges/challenge-list.html',
        controller: 'ChallengeListCtrl'
      });
  });
