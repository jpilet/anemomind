'use strict';

angular.module('www2App')
  .controller('BoatadminCtrl', function ($scope, Auth, boatList, BoxExec, $q) {
    $scope.isAdmin = Auth.isAdmin;

    $scope.remoteCommand = '';
    $scope.ugradeVersion = undefined;

    $scope.boats = boatList.boats();
    $scope.$on('boatList:updated', function(event, boats) {
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

    $scope.updateBoats = function() {
    }

  });
