'use strict';

angular.module('www2App')
  .controller('ChallengeListCtrl', function ($scope, $http, socket) {
      $scope.sharePopup = {
        content: 'Share my track',
        templateUrl: 'shareTemplate.html',
        title: 'Share my track'
      };
  });
