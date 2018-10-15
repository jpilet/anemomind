'use strict';

angular.module('www2App')
  .config(function ($stateProvider) {
    $stateProvider
      .state('login', {
        url: '/login',
        templateUrl: 'app/account/login/login.html',
        controller: 'LoginCtrl'
      })
      .state('signup', {
        url: '/signup?email&name',
        templateUrl: 'app/account/signup/signup.html',
        controller: 'SignupCtrl'
      })
      .state('settings', {
        url: '/settings',
        templateUrl: 'app/account/settings/settings.html',
        controller: 'SettingsCtrl',
        authenticate: true
      })
      .state('resetpassword', {
        url: '/resetpassword',
        templateUrl: 'app/account/resetpassword/resetpassword.html',
        controller: 'ResetPasswordCtrl',
        authenticate: false
      });
  });
