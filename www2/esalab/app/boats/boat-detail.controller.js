'use strict';

angular.module('www2App')
  .controller('BoatDetailCtrl', function ($scope, $stateParams, Auth, $http, userDB, boatList, $location) {
    $scope.boat = { sails: [] };
    $scope.invitationMessage = "";
    $scope.invitedEMail = "";
    $scope.invitedAdmin = true;
    $scope.users = {};
    $scope.newSails = "";
    $scope.isAdmin = Auth.isAdmin;
    $scope.boatId=$stateParams.boatId;

    var resolveUser = function(user) {
      userDB.resolveUser(user, function(user) {
        $scope.users[user._id] = user;
      });
    };

    boatList.boat($stateParams.boatId)
      .then(function (boat) {
        $scope.boat = boat;
        boat.admins.forEach(resolveUser);
        boat.readers.forEach(resolveUser);
      });


    $scope.saveBoat = function(navigateAway) {
      boatList.save($stateParams.boatId, $scope.boat)
        .success(function(boat) { 
          $scope.boat = boat; 
          if (navigateAway) {
            $location.path('/boats/' + boat._id);
          }
        });
    }

    $scope.addMember = function() {
      var invitation = {
        email: $scope.invitedEMail,
        admin: $scope.invitedAdmin
      };

      boatList.addMember($stateParams.boatId,invitation)
      .success(function(data, status, header, config) {
        if (data.message) {
          $scope.invitationMessage = data.message;
        }
        if (data.user) {
          $scope.users[data.user._id] = data.user;
        }
        if (data.boat) {
          $scope.boat = data.boat;
        }
        $scope.invitedEMail = "";
      })
      .error(function(err) {
         $scope.invitationMessage = err;
      });
    };

    $scope.removeMember = function(field, entry) {
      var entryAsJson = angular.toJson(entry);
      $scope.boat[field] = jQuery.grep($scope.boat[field], function(x) {
        return angular.toJson(x) != entryAsJson;
      });
      $scope.saveBoat();
    };

    $scope.removeInvitation = function(guest) {
      $scope.removeMember('invited', guest);
    };

    $scope.removeSail = function(sail) {
      var index = $scope.boat.sails.indexOf(sail);
      $scope.boat.sails.splice(index, 1);
    };

    $scope.addSail = function() {
      if ($scope.newSail) {
        $scope.boat.sails.push($scope.newSail);
        $scope.newSail = "";
      }
    };

    $scope.changePublicAccess = function() {
      $scope.boat.publicAccess = ! $scope.boat.publicAccess;
      $scope.saveBoat();
    };

  });
