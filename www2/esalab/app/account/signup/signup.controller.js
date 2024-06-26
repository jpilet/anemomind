'use strict';

angular.module('www2App')
  .controller('SignupCtrl', function ($scope, Auth, $location, $window, $stateParams) {
    $scope.user = {};
    $scope.errors = {};

    if ($stateParams.email) {
      $scope.user.email = $stateParams.email;
    }
    if ($stateParams.name) {
      $scope.user.name = $stateParams.name;
    }

    $scope.register = function(form) {
      $scope.submitted = true;

      if(form.$valid) {
        Auth.createUser({
          name: $scope.user.name,
          email: $scope.user.email,
          password: $scope.user.password
        })
        .then( function() {
          // Account created, redirect to home
          $location.path('/');
        })
        .catch( function(err) {
          err = err.data;
          $scope.errors = {};

          if (err.code == 11000) {
            form.email.$setValidity('alreadyRegistered', false);
            $scope.errors.email = "e-mail address already registered";
          } else if (err.errors) {
            // Update validity of form fields that match the mongoose errors
            angular.forEach(err.errors, function(error, field) {
              form[field].$setValidity('mongoose', false);
              $scope.errors[field] = error.message;
            });
          }
        });
      }
    };

    $scope.loginOauth = function(provider) {
      $window.location.href = '/auth/' + provider;
    };
  });
