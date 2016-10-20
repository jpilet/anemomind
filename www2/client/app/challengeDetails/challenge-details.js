'use strict';

angular.module('www2App')
  .config(function ($stateProvider) {
    $stateProvider
      .state('challenge-details', {
        //TODO: add parameter for userId
        url: '/challenge/details',
        templateUrl: 'app/challengeDetails/challenge-details.html',
        controller: 'ChallengeDetailsCtrl'
      });
  });
