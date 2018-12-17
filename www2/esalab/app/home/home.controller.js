
angular.module('www2App')
  .controller('HomeCtrl', function ($scope, $http, $stateParams,$location, socket, boatList,Auth,$log) {
    $scope.isLoggedIn = Auth.isLoggedIn();
    $scope.loading = true;

    $scope.$watch(Auth.isLoggedIn, function(newVal, oldVal) {
      $scope.isLoggedIn = newVal;
      update();
    });

    var update = function() {
      // TODO: only load the boat we want, not all boats.
      boatList.boats().then(function(boats) {
        $scope.boats = boats;

        $scope.loading = false;

        var boat = (boatList.getDefaultBoat() || {})._id;        

        // The home page when not logged in is the invite to log in.
        // Otherwise, we need to redirect...
        if (Auth.isLoggedIn()) {
          if (boat) {
            // ... to the default boat, if any
            $location.path('/boats/' + boat);
          } else {
            // ... or to the boat creation page
            $location.path('/boats');
          }
        }
      });
    };

    update();
  });
