;(function(angular) {'use strict';


//
// Define the application level controllers
angular.module('app.root', [])
  .controller('AppCtrl',appCtrl);

appCtrl.$inject=[
  '$scope','$rootScope','$window','$location','$routeParams','$timeout','$http','$translate'];
function appCtrl($scope, $rootScope, $window,  $location, $routeParams, $timeout, $http, $translate) {


  $scope.options={
    needReload:false,
    locale:$translate.use()
  };



  $scope.locale=function () {
    return $scope.options.locale;
  };

  $scope.changeLanguage = function (langKey) {
    $translate.use(langKey);
    $scope.options.locale=langKey;
    // update server
    // $http.get(config.API_SERVER+'/v1/config?lang='+langKey);    
  };



}

})(window.angular);