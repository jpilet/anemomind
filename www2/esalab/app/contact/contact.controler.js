angular.module('www2App')
  .controller('ContactCtrl', function ($scope, $http, Auth) {
    $scope.isLoggedIn = Auth.isLoggedIn();
    $scope.loading = true;
    $scope.form = {
      name: "",
      subject: "",
      addr: "",
      message: ""
    };
    $scope.messageSent = false;
    $scope.messageError = undefined;

    var emailRegex = /^(([^<>()\[\]\\.,;:\s@"]+(\.[^<>()\[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;

    var update = function() {
      if ($scope.isLoggedIn) {
        var user = Auth.getCurrentUser();
        $scope.form.name = user.name;
        $scope.form.addr = user.email;
      }
    };

    $scope.$watch(Auth.isLoggedIn, function(newVal, oldVal) {
      $scope.isLoggedIn = newVal;
      update();
    });
    update();

    $scope.sendMessage = function() {
      if ($scope.form.name.length < 3) {
        $scope.messageError = "Please enter your name";
        return;
      }
      if ($scope.form.subject.length < 3) {
        $scope.messageError = "Please enter a subject";
        return;
      }
      if ($scope.form.message.length < 10) {
        $scope.messageError = "Please enter a message";
        return;
      }
      if (!$scope.isLoggedIn && !$scope.form.addr.match(emailRegex)) {
        $scope.messageError = "Please enter a valid email address";
        return;
      }

      $scope.messageError = "";
      $scope.messageSent = false;

      $http.post("/api/contact", $scope.form)
      .then(function(arg) {
          $scope.messageError = undefined;
          $scope.messageSent = true;
        }, function(err) {
          $scope.messageError = "Can't send message";
          $scope.messageSent = false;
        });
    };
  });
