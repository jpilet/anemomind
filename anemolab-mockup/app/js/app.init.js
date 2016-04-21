;(function(angular) {'use strict';

// Declare application level module which depends on additional filters and services (most of them are custom)
angular.module('app', [
  'ngCookies',
  'ngResource',
  'ngRoute',
  'ngSanitize',
  'ngTouch',
  'ngAnimate',
  'pascalprecht.translate',
  'flash',
  'angular.filter',
  'app.storage',
  'app.root'
])
  .run(appRun);



//
// Scroll events can be triggered very frequently, which can hurt performance and make scrolling appear jerky.
//angular.module('infinite-scroll').value('THROTTLE_MILLISECONDS', 500)

//
// init the module
appRun.$inject=[ '$route', '$http','$timeout'];
function appRun( $route, $http, $timeout) {
  // Application is ready there
  angular.element('html').removeClass('app-loading');


}


// Bootstrap (= launch) application
angular.element(document).ready(function () {

  //
  // loading fastclick for mobile tap
  FastClick.attach(document.body);


  //console.log(window.Showdown.extensions)
  angular.bootstrap(document, ['app']);
});

})(window.angular);
