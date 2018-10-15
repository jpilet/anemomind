'use strict';

angular.module('www2App')
  .controller('BoatadminCtrl', function ($scope, Auth, boatList, BoxExec, $q, $resource, $http) {
    $scope.isAdmin = Auth.isAdmin;

    $scope.remoteCommand = '';

    // $scope.boats = boatList.boats();
    boatList.boats().then(function(boats) {
       $scope.boats = boats;
    });

    $scope.toggleAll = function() {
      for (var i in $scope.boats) {
        var boat = $scope.boats[i];
        boat.selected = !boat.selected;
      }
    };

    $scope.setSelectionAll = function(yesno) {
      for (var i in $scope.boats) {
        $scope.boats[i].selected = !!yesno;
      }
    };

    $scope.delete = function(boat) {
      // TODO
    };

    $scope.sendCommand = function(type) {
      var promises = [];

      for (var i in $scope.boats) {
        var boat = $scope.boats[i];

        if (!boat.selected) {
          continue;
        }

        var boxexec = new BoxExec({
          boatId: boat._id,
          scriptType: type,
          scriptData: $scope.remoteCommand
        });
        promises.push(boxexec.$save());
      }

      $q.all(promises).then(
        function(e) {
          $scope.remoteCommand = '';
          alert("Sent.");
        }, function(err) {
          if (err && err.data && err.data.message) {
            alert(err.data.message);
          } else {
            alert('Error: ' + JSON.stringify(err));
          }
        }
      );
    };

    var Updates = $resource('/api/updates/');

    $scope.updates = Updates.query({});
    $scope.selectedUpdate = undefined;
    $scope.updateStatus = "";


    $scope.sendUpdate = function() {
      if (!$scope.selectedUpdate || !$scope.selectedUpdate.name) {
        $scope.updateStatus = "Please select an update to send";
        return;
      }

      $scope.updateStatus = 'Sending ' + $scope.selectedUpdate + '...<br/>';

      var query = { update: $scope.selectedUpdate.name, boats: [] };

      for (var i in $scope.boats) {
        var boat = $scope.boats[i];
        if (boat.selected) {
          query.boats.push(boat._id);
        }
      }

      $scope.updateStatus = 'Sending update...';
      $http.post('/api/updates/sendupdate', query)
        .then(
            function(response) {
              $scope.updateStatus = 'Success: ' + response.data;
            },
            function(response) {
              $scope.updateStatus = 'Error: ' + response.data;
            }
          );
    };

  });
