'use strict';

angular.module('www2App')
  .controller('ChallengesCtrl', function ($scope, $http, socket, Auth) {
      $scope.isAdmin = Auth.isAdmin;
      $scope.isLoggedIn = Auth.isLoggedIn();

      $scope.challenges = [
        {
          'name': 'Fastest Distance',
          'description': "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Mauris odio nunc, accumsan vitae tristique faucibus, vulputate ut turpis."
        },
        {
          'name': 'Fastest period of time',
          'description': "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Mauris odio nunc, accumsan vitae tristique faucibus, vulputate ut turpis."
        },
        {
          'name': 'Monocup Challenge',
          'description': "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Mauris odio nunc, accumsan vitae tristique faucibus, vulputate ut turpis."
        },
        {
          'name': '500m Challenge',
          'description': "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Mauris odio nunc, accumsan vitae tristique faucibus, vulputate ut turpis."
        },
        {
          'name': '1h Challenge',
          'description': "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Mauris odio nunc, accumsan vitae tristique faucibus, vulputate ut turpis."
        }
      ];
  });
