'use strict';

angular.module('www2App')
  .config(function ($stateProvider) {
    $stateProvider
      .state('session', {
        url: '/session/:sessionId',
        templateUrl: 'app/session/session.html',
        controller: 'SessionCtrl'
      });
  });
