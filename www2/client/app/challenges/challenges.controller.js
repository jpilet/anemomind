'use strict';

angular.module('www2App')
  .controller('ChallengesCtrl', function ($scope, $http, socket, Auth, ModalService) {
      $scope.isAdmin = Auth.isAdmin;
      $scope.isLoggedIn = Auth.isLoggedIn();

      $scope.selectedChallenge = null;
      $scope.reverse = true;
      $scope.defaultOrder = 'rank';
      $scope.challenges = [
        {
          name: '200 meters',
          classes: 'distance'
        },
        {
          name: '500 meters',
          classes: 'distance'
        },
        {
          name: '1 km',
          classes: 'distance'
        },
        {
          name: '5 km',
          classes: 'distance'
        },
        {
          name: '20 km',
          classes: 'distance'
        },
        {
          name: '10 minutes',
          classes: 'time minutes10'
        },
        {
          name: '30 minutes',
          classes: 'time minutes30'
        },
        {
          name: '1 hour',
          classes: 'time hour1'
        },
        {
          name: '5 hours',
          classes: 'time hours5'
        },
        {
          name: '24 hours',
          classes: 'time hours24'
        },
      ];
      $scope.challengeList = [
        {
          rank: 1,
          name: 'Larry Smith',
          boat: 'Irene',
          type: 'Sailboat',
          class: '6 MR',
          sailNumber: 'SUI 91',
          length: '11 meters',
          speed: 4.2,
          distance: '500 m',
          speedUnit: 'kn',
          startTime: '2016-08-25T11:09:36.376Z',
          endTime: '2016-08-25T14:29:22.607Z', 
        },
        {
          rank: 2,
          name: 'Thornton Thompson',
          boat: 'Velocissima',
          type: 'Sailboat',
          class: '6 MR',
          sailNumber: 'SUI 91',
          length: '11 meters',
          speed: 5,
          distance: '500 m',
          speedUnit: 'kn',
          startTime: '2016-08-28T12:14:32.833Z',
          endTime: '2016-08-28T15:49:08.463Z', 
        },
        {
          rank: 3,
          name: 'John Doe',
          boat: 'Dentuso',
          type: 'Sailboat',
          class: '6 MR',
          sailNumber: 'SUI 91',
          length: '11 meters',
          speed: 3.5,
          distance: '500 m',
          speedUnit: 'kn',
          startTime: '2016-09-04T11:27:33.175Z',
          endTime: '2016-09-04T15:06:33.827Z', 
        }         
      ];
      
      $scope.sharePopup = {
        content: '',
        templateUrl: 'shareTemplate.html',
        title: ''
      };
      $scope.fillDetails = function(data) {
        $scope.detail = {
          boat: data.boat,
          class: data.class,
          sailNumber: data.sailNumber,
          length: data.length,
        };
      }

      $scope.sortBy = function(sort) {
        $scope.reverse = ($scope.defaultOrder === sort) ? !$scope.reverse : false;
        $scope.defaultOrder = sort;
      };

      $scope.selectChallenge = function(index, challenge) {
        $scope.selectedChallenge = index;
        $scope.challengeName = challenge.name;
      }

      $scope.showModal = function(data) { 
        ModalService.data = data;
        ModalService.showModal({
          templateUrl: "app/share/share.html",
          controller: "ShareCtrl",
          inputs: {
            name: "Fry",
            year: 3001
          }
        });
      }

      // Responsive config for the Slick slider
      $scope.breakpoints = [
        {
          breakpoint: 768,
          settings: {
            slidesToShow: 3
          }
        },
        {
          breakpoint: 480,
          settings: {
            slidesToShow: 2
          }
        },
        {
          breakpoint: 380,
          settings: {
            slidesToShow: 1
          }
        }
      ];
  });
