'use strict';

angular.module('www2App')
  .controller('ChallengeListCtrl', function ($scope, $http, socket) {
      $scope.sharePopup = {
        content: 'Share my track',
        templateUrl: 'shareTemplate.html',
        title: 'Share my track'
      };

      $scope.propertyName = 'rank';
      $scope.reverse = true;
      $scope.challengeList = [
        {
          'rank': 1,
          'name': 'Larry Smith',
          'boat': 'Irene',
          'type': 'Sailboat',
          'speed': 4.2,
          'speedUnit': 'kn',
          'startTime': '2016-08-25T11:09:36.376Z',
          'endTime': '2016-08-25T14:29:22.607Z', 
        },
        {
          'rank': 2,
          'name': 'Thornton Thompson',
          'boat': 'Velocissima',
          'type': 'Sailboat',
          'speed': 5,
          'speedUnit': 'kn',
          'startTime': '2016-08-28T12:14:32.833Z',
          'endTime': '2016-08-28T15:49:08.463Z', 
        },
        {
          'rank': 3,
          'name': 'John Doe',
          'boat': 'Dentuso',
          'type': 'Sailboat',
          'speed': 3.5,
          'speedUnit': 'kn',
          'startTime': '2016-09-04T11:27:33.175Z',
          'endTime': '2016-09-04T15:06:33.827Z', 
        }         
      ];

      

      $scope.sortBy = function(propertyName) {
        $scope.reverse = ($scope.propertyName === propertyName) ? !$scope.reverse : false;
        $scope.propertyName = propertyName;
      };
  });
