'use strict';

angular.module('www2App')
  .filter('timeSpan', function () {
      return function(span) {
          var fromDate = new Date(span[0]);
          var toDate = new Date(span[1]);

/* 
Alternative implementation from http://stackoverflow.com/questions/20196113/angularjs-how-do-you-convert-milliseconds-to-xhours-and-ymins
There are some minor differences between the strings that the two versions produce.

        function makeDurString2(seconds) {
          var days = Math.floor(seconds / 86400);
          var hours = Math.floor((seconds % 86400) / 3600);
          var minutes = Math.floor(((seconds % 86400) % 3600) / 60);
          var timeString = '';
          if(days > 0) timeString += (days > 1) ? (days + " days ") : (days + " day ");
          if(hours > 0) timeString += (hours > 1) ? (hours + " hours ") : (hours + " hour ");
          if(minutes >= 0) timeString += (minutes > 1) ? (minutes + " minutes ") : (minutes + " minute ");
          return timeString;
        }
*/
          function makeDurString(durSeconds) {
            var dur = durSeconds;
            var multipliers = [60, 60, 24, 7, 1000000000];
            var labels = ['second', 'minute', 'hour', 'day', 'week'];
            var formatDur = function(x, s) {
              return (x == 0? null : x + ' ' + s + (x == 1? "" : "s"));
            }
            var dst = [];
            for (var i = 0; i < multipliers.length; i++) {
              var f = multipliers[i];
              var a = Math.floor(dur/f);
              var b = Math.floor(dur - f*a);
              var s = formatDur(b, labels[i]);
              if (s) {
	              dst.push(s);
              }
              dur = a;
            }
            var finalParts = dst.reverse();
            if (finalParts.length == 0) {
              return "0 seconds";
            } else if (finalParts.length == 1) {
	            return finalParts[0];
            } else {
              return finalParts[0] + ', ' + finalParts[1];
            }
          }

          var offsetIso = fromDate.toISOString();

          // What is both an easily readable and 
          // somewhat standard format for dates? 
          // Raw ISO dates are standard,
          // but look a bit technical in this context, 
          // so let's declutter it a bit...
          var offsetString = offsetIso.substring(0, 10) 
              + " " + offsetIso.substring(11, 16);
          var durSeconds = 0.001*(toDate.getTime() - fromDate.getTime());
          return offsetString + " (" + makeDurString(durSeconds) + ")";
      };
  });
