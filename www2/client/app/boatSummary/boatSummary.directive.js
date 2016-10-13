'use strict';

var app = angular.module('www2App')
  .directive('boatSummary', function ($rootScope, $location, $interpolate, boatList, Auth, $log) {
    var storedPageSize={};

    return {
      templateUrl: 'app/boatSummary/boatSummary.html',
      restrict: 'E',
      scope: {
        boatId: "=",
        pageSize: "=?"
      },
      link: function (scope, element, attrs) {

        scope.currentPage = 1;
        scope.sessions = [];

        function updateSessions() {

          if(!scope.boatId) return;

          scope.pageSize=storedPageSize[scope.boatId]||2;
          //
          // be sure that boats are ready
          $log.log('-- boatSummary.update.0');
          boatList.boats().then(function (boats) {
            $log.log('-- boatSummary.update',boats.length);

            scope.boat = boatList.boat(scope.boatId);
            scope.sessions = boatList.sessionsForBoat(scope.boatId);

            //
            // ensure that sessions is not empty
            scope.sessions=scope.sessions||[];

            scope.globalMaxSpeed = undefined;
            scope.globalMaxSpeedTime = undefined;
            scope.globalMaxSpeedSession = undefined;

            scope.sessions.forEach(function(session) {
              session.hasPhoto=scope.hasSocialActivity(session,'photos');
              session.hasComment=scope.hasSocialActivity(session,'comments');

              var maxSpeed = session.maxSpeedOverGround;
              if (!isNaN(maxSpeed) && maxSpeed > 1.0) {
                if (!scope.globalMaxSpeed
                    || scope.globalMaxSpeed < session.maxSpeedOverGround) {
                  scope.globalMaxSpeed = session.maxSpeedOverGround;
                  scope.globalMaxSpeedTime = session.maxSpeedOverGroundTime;
                  scope.globalMaxSpeedSession = session;
                }
              }
            })
          });

        }



        // Why those listeners?
        // scope.$on('boatList:updated', updateSessions);
        // scope.$on('boatList:sessionsUpdated', updateSessions);
        scope.$watch('boatId', updateSessions);


        //
        // display more session
        scope.showMoreSessions=function(){
          scope.pageSize+=10;
          storedPageSize[scope.boatId]=scope.pageSize;
        }

        //
        // get photo here
        // TODO make this a directive or an API available on top controller
        scope.photoUrl = function(bid,photo, size) {
          return '/api/events/photo/' + bid + '/' + photo
            + '?' + (size? 's=' + size + '&' : '') + 'access_token=' + Auth.getToken() ;
        };

        //
        // Used when loading new sessions
        scope.hasSocialActivity=function(session,field){
          if(!scope.boat[field]||!scope.boat[field].length){
            return false;
          }
          var start=new Date(session.startTime);
          var end=new Date(session.endTime);

          return (scope.boat[field].filter(function(elem) {
            var when=new Date(elem.when)
            return start<=when&&end>=when;
          }).length>0);

        };



        //
        // get first session date
        // TODO this could be a directive 
        scope.sessionGetFirstDate=function(sessions) {
          return sessions.length&&sessions[0].startTime;
        };
        //
        // compute avg in current sessions for direction
        // TODO this could be a directive 
        scope.sessionTotalDistance=function (sessions) {
          var sum = sessions.reduce(function(prev, session) { 
            return prev + session.trajectoryLength; 
          },0);
          return sum.toFixed(1);
        };

        //
        // compute avg in current sessions for speed
        // TODO this could be a directive 
        scope.sessionAvgSpeed=function (sessions) {
          var sum = sessions.reduce(function(prev, session) { 
            return prev + session.avgWindSpeed; 
          },0);
          return (sum / sessions.length).toFixed(1);
        };

        //
        // compute avg in current sessions for speed
        // TODO this could be a directive 
        scope.sessionMaxBoatSpeed=function (sessions) {
          var maxSpeed = Math.max.apply(Math,sessions.map(function(session){
            return session.maxSpeedOverGround;
          }));
          if (isNaN(maxSpeed) || maxSpeed < 1) {
            return "N/A";
          }
          return maxSpeed.toFixed(1);
        };




        scope.twdirToCardinal = function(twdir) {
          var index = Math.round(360 + twdir * 8 / 360) % 8;
          var windrose = [
            "N", "NE", "E", "SE", "S", "SW", "W", "NW" ];
          return windrose[index];
        };

        scope.knotsToBeaufort = function(knots) {
          if (knots < 1) { return 0; }
          if (knots < 3) { return 1; }
          if (knots < 6) { return 2; }
          if (knots < 10) { return 3; }
          if (knots < 16) { return 4; }
          if (knots < 21) { return 5; }
          if (knots < 27) { return 6; }
          if (knots < 33) { return 7; }
          if (knots < 40) { return 8; }
          if (knots < 47) { return 9; }
          if (knots < 55) { return 10; }
          if (knots < 63) { return 11; }
          return 12;
        };

        scope.timeToInt = function(time) {
          return new Date(time).getTime();
        };
      }
    };
  });


//
// adapt text size depending on container space
app.directive('boatFixTitleSize',['$timeout',function($timeout) {
  return {
    restrict:'A',
    replace:false,
    link:function(scope,element,attrs) {
      //
      // simple way with $timeout 
      // in case of trouble use,             
      // attrs.$observe('boatFixTitleSize', function(name) {
      $timeout(function() {
        var marginLeft=40;
        var ourText = element.find('.fixed-size');
        var fontSize = parseInt(window.getComputedStyle(ourText[0], null).getPropertyValue('font-size'));
        var maxWidth = element.width();
        var textWidth= ourText.width();
        while ((textWidth > (maxWidth-marginLeft)) && fontSize > 12){
            ourText.css('font-size', --fontSize);
            textWidth = ourText.width();
        };
      },500);

    }
  };
}]);

app.directive('boatMainImage', ['$parse', function($parse) {
  var style={
    'background-size':'cover',
    'background-color':'transparent',
    'background-position': '50% 20%',
    'height':'100%'
  }, defaultImage="/assets/images/boat-sample.jpg";    

  return {
    restrict: 'A',
    replace: false, 
    scope:{
      boatMainImage:'='
    },
    link: function(scope, element, attrs, ngModelCtrl) {
      var self=this;

      scope.$watch('boatMainImage', function (boatMainImage) {
        
        //
        // initial value
        style['background-image']='url('+defaultImage+')';
        if(boatMainImage&&
           boatMainImage._id&&
           boatMainImage.photos&&
           boatMainImage.photos.length){
          var path=scope.$parent.photoUrl(boatMainImage._id,boatMainImage.photos[0].src,'300x400');
          style['background-image']='url('+path+')';
        }
        element.css(style);
      });

      scope.timeToInt = function(time) {
        return new Date(time).getTime();
      };
    }
  }
}]);



app.filter('startFrom', function() {
    return function(input, start) {
        start = +start; //parse to int
        if (input) return input.slice(start);
    }
});
