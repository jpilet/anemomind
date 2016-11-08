'use strict';
  
//
// directive that display html element if user is admin of the current boat 
angular.module('www2App')
  .directive('showIfBoatadmin', function ( $window, boatList, Auth) {
    return {
      restrict: 'A',
      link: function (scope, element, attrs) {
        //
        // default action is driven by CSS
        var defaultClass="ng-hide", successAction="ng-show";
 
        attrs.$observe('showIfBoatadmin',function(boatId) {
          element.addClass(defaultClass);
          if(!boatId){
            // not ready yet
            return;
          }

          scope.user=Auth.getCurrentUser();
          boatList.boats().then(function(boats) {
            var boat=boatList.boat(boatId);
            
            if(!!~boat.admins.indexOf(scope.user._id)){
              element.addClass(successAction).removeClass(defaultClass);
            }

          });
        });
      }
    };
  })
