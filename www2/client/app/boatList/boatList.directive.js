'use strict';
  
//
// directive that display html element if user is admin of the current boat 
angular.module('www2App')
  .directive('showIf', function ( $window,$parse, boatList, Auth) {
    return {
      restrict: 'A',
      link: function (scope, element, attrs) {
        //
        // default action is driven by CSS
        var defaultClass="ng-hide", 
            successAction="ng-show",
            admin=false,
            showIf=false;

        attrs.$observe('showBoat',function(boat) {
          element.addClass(defaultClass);
          showIf=attrs.showIf;
          boat=$parse('boat')(scope);
          scope.user=Auth.getCurrentUser();
          admin=(boat.admins && boat.admins.indexOf(scope.user._id)>-1);

          switch (attrs.showIf){
          case 'admin':
            showIf=admin;
            break;
          case '!admin':
            showIf=!admin;
            break;
          case '!admin+!public':
            showIf=!admin && !boat.publicAccess;
            break;
          case '!admin+public':
            showIf=!admin && boat.publicAccess;
            break;
          case 'admin+!public':
            showIf=admin && !boat.publicAccess;
            break;
          case 'admin+public':
            showIf=admin && boat.publicAccess;
            break;
          }
          
          showIf && element.addClass(successAction).removeClass(defaultClass);
        });
      }
    };
  })

//
// directive that HIDE  html element if user is admin of the current boat 
  .directive('hideIf', function ( $window,$parse, boatList, Auth) {
    return {
      restrict: 'A',
      link: function (scope, element, attrs) {
        //
        // default action is driven by CSS
        var defaultClass="ng-show", 
            successAction="ng-hide",
            admin=false,
            hideIf=false;

        attrs.$observe('hideBoat',function(boat) {
          element.addClass(defaultClass);
          hideIf=attrs.hideIf;
          boat=$parse('boat')(scope);
          scope.user=Auth.getCurrentUser();
          admin=(boat.admins.indexOf(scope.user._id)>-1);

          switch (attrs.hideIf){
          case '!admin+!public':
            hideIf=!admin && !boat.publicAccess;
            break;
          case '!admin+public':
            hideIf=!admin && boat.publicAccess;
            break;
          case 'admin+!public':
            hideIf=admin || !boat.publicAccess;
            break;
          case 'admin+public':
            hideIf=admin || boat.publicAccess;
            break;
          }
          
          hideIf && element.addClass(successAction).removeClass(defaultClass);
        });
      }
    };
  })

