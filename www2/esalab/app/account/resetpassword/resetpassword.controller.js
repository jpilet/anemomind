'use strict';

angular.module('www2App')
  .controller('ResetPasswordCtrl', function ($scope, User, Auth, $state) {
    $scope.errors = {};

    $scope.resetPassword = function(form) {
      $scope.submitted = true;
      if(form.$valid) {
        $scope.submittedValid = true;
        Auth.resetPassword( $scope.user.email )
        .then( function() {
          $scope.message = 'A new password has been sent to your email address.';
        })
        .catch( function() {
          form.email.$setValidity('mongoose', false);
          $scope.errors.other = 'Unknown email.';
          $scope.message = '';
        });
      }
		};
  });
