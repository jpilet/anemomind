'use strict';

angular.module('www2App')
  .controller('BoatDetailCtrl', function ($scope, $stateParams, Auth, $http, userDB) {
    $scope.boat = { sails: [] };
    $scope.invitationMessage = "";
    $scope.invitedEMail = "";
    $scope.invitedAdmin = true;
    $scope.users = {};
    $scope.newSails = "";
    $scope.isAdmin = Auth.isAdmin;

    var resolveUser = function(user) {
      userDB.resolveUser(user, function(user) {
        $scope.users[user._id] = user;
      });
    };

    $http.get('/api/boats/' + $stateParams.boatId)
    .success(function(data, status, headers, config) {
      $scope.boat = data;
      for (var i in data.admins) {
        resolveUser(data.admins[i]);
      }
      for (var i in data.readers) {
        resolveUser(data.readers[i]);
      }
    });

    $scope.saveBoat = function() {
      $http.put('/api/boats/' + $stateParams.boatId, $scope.boat)
      .success(function(data) { $scope.boat = data; });
    }

    $scope.addMember = function() {
      var invitation = {
        email: $scope.invitedEMail,
        admin: $scope.invitedAdmin
      };

      $http.put('/api/boats/' + $stateParams.boatId + '/invite', invitation)
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
    }
    $scope.addSail = function() {
      if ($scope.newSail) {
        $scope.boat.sails.push($scope.newSail);
        $scope.newSail = "";
      }
    }
  });
