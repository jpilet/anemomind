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
            var precedingGap = 0;
            for (var i = 0; i < multipliers.length; i++) {
              var f = multipliers[i];
              var a = Math.floor(dur/f);
              var b = Math.floor(dur - f*a);
              var s = formatDur(b, labels[i]);
              if (s) {
	              dst.push({value: s, gap: precedingGap});
                precedingGap = 0;
              } else {
                precedingGap++;
              }
              dur = a;
            }
            var finalParts = dst.reverse();
            if (finalParts.length == 0) {
              return "0 seconds";
            } else {
              var first = finalParts[0];
              if (finalParts.length == 1 || first.gap > 0) {
                return first.value;
              } else {
                return first.value + ', ' + finalParts[1].value;
              }
            }
          }

        function formatDateTime(d) {
          // Since Javascript doesn't come with some good date
          // and time formatting utilities, hack something
          // that will display it in a standardized way of the local time
          // zone of the browser. Ideally, we should use the time zone where
          // the session took place.
          var tweakedDate = new Date(d.getTime() - d.getTimezoneOffset()*60*1000);

          var offsetIso = tweakedDate.toISOString();
          return offsetIso.substring(0, 10) 
              + " " + offsetIso.substring(11, 16);
        }

        var durSeconds = 0.001*(toDate.getTime() - fromDate.getTime());
        return formatDateTime(fromDate) + " (" + makeDurString(durSeconds) + ")";
      };
  });
