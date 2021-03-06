'use strict';

angular.module('www2App', [
  'ngCookies',
  'ngResource',
  'ngSanitize',
  'btford.socket-io',
  'ui.router',
  'ui.bootstrap',
  'angularFileUpload',
  'nvd3',
  'bootstrapLightbox',
  'rzModule',
  'angularModalService'
])
  .config(function ($stateProvider, $urlRouterProvider, $locationProvider, $httpProvider, LightboxProvider) {
    $urlRouterProvider
      .otherwise('/');

    // IE Caching issue for $http
    // - http://benjii.me/2014/07/ie-is-caching-my-angular-requests-to-asp-net-mvc/
    // - http://stackoverflow.com/questions/16098430/angular-ie-caching-issue-for-http
    //
    // Initialize get if not there
    if (!$httpProvider.defaults.headers.get) {
        $httpProvider.defaults.headers.get = {};
    }

    // Enables Request.IsAjaxRequest() in ASP.NET MVC
    $httpProvider.defaults.headers.common["X-Requested-With"] = 'XMLHttpRequest';

    // Disable IE ajax request caching
    $httpProvider.defaults.headers.get['If-Modified-Since'] = '0';

    $locationProvider.html5Mode(true);
    $httpProvider.interceptors.push('authInterceptor');

    LightboxProvider.fullScreenMode = true;
  })

  .factory('authInterceptor', function ($rootScope, $q, $cookieStore, $location) {
    return {
      // Add authorization token to headers
      request: function (config) {
        config.headers = config.headers || {};
        var token=$cookieStore.get('token');

        if (token) {
          config.headers.Authorization = 'Bearer ' + token;
        }
        return config;
      },

      // Intercept 401s and redirect you to login
      responseError: function(response) {
        if(response.status === 401) {
          var url = $location.url();
          var redirect = '';
          // If the user tries to access /login or /, we do not need a redirect.
          // We need the redirection only when the user attempts to access a
          // real page.
          if (url.match(/^\/(boats|map|vmgplot)\//)) {
            redirect = '?d=' + encodeURIComponent(url);
          }
          $location.url('/login' + redirect);
          // remove any stale tokens
          $cookieStore.remove('token');
          return $q.reject(response);
        }
        else {
          return $q.reject(response);
        }
      },
    };
  })

  .run(function ($rootScope, $location, Auth, boatList,$log) {

    // Redirect to login if route requires auth and you're not logged in
    $rootScope.$on('$stateChangeStart', function (event, next) {
      Auth.isLoggedInAsync(function(loggedIn) {
        if (next.authenticate && !loggedIn) {
          $location.path('/login');
        }
      });
    });
  });
