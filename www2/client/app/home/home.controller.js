
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

        // The home page for a logged in user with a valid default boat
        // is the default boat page. The redirection occurs here.
        if (Auth.isLoggedIn() && boat) {
            $location.path('/boats/' + boat);
        }
      });
    };

    update();
  });
