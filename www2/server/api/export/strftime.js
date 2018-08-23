/* Port of strftime(). Compatibility notes:
 *
 * %c - formatted string is slightly different
 * %D - not implemented (use "%m/%d/%y" or "%d/%m/%y")
 * %e - space is not added
 * %E - not implemented
 * %h - not implemented (use "%b")
 * %k - space is not added
 * %n - not implemented (use "\n")
 * %O - not implemented
 * %r - not implemented (use "%I:%M:%S %p")
 * %R - not implemented (use "%H:%M")
 * %t - not implemented (use "\t")
 * %T - not implemented (use "%H:%M:%S")
 * %U - not implemented
 * %W - not implemented
 * %+ - not implemented
 * %% - not implemented (use "%")
 *
 * strftime() reference:
 * http://man7.org/linux/man-pages/man3/strftime.3.html
 *
 * Day of year (%j) code based on Joe Orost's answer:
 * http://stackoverflow.com/questions/8619879/javascript-calculate-the-day-of-the-year-1-366
 *
 * Week number (%V) code based on Taco van den Broek's prototype:
 * http://techblog.procurios.nl/k/news/view/33796/14863/calculate-iso-8601-week-and-year-in-javascript.html
 */
module.exports = function (sFormat, date) {
  if (!(date instanceof Date)) date = new Date();
  var nDay = date.getUTCDay(),
    nDate = date.getUTCDate(),
    nMonth = date.getUTCMonth(),
    nYear = date.getUTCFullYear(),
    nHour = date.getUTCHours(),
    aDays = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'],
    aMonths = ['January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December'],
    aDayCount = [0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334],
    isLeapYear = function() {
      return (nYear%4===0 && nYear%100!==0) || nYear%400===0;
    },
    getThursday = function() {
      var target = new Date(date);
      target.setDate(nDate - ((nDay+6)%7) + 3);
      return target;
    },
    zeroPad = function(nNum, nPad) {
      return ('' + (Math.pow(10, nPad) + nNum)).slice(1);
    };
  return sFormat.replace(/%[a-z]/gi, function(sMatch) {
    switch(sMatch) {
      case '%m': return zeroPad(nMonth + 1, 2);
      case '%d': return zeroPad(nDate, 2);
      case '%Y': return nYear;
      case '%l': return (nHour+11)%12 + 1;
      case '%M': return zeroPad(date.getUTCMinutes(), 2);
      case '%S': return zeroPad(date.getUTCSeconds(), 2);
      case '%p': return (nHour<12) ? 'AM' : 'PM';
      case '%I': return zeroPad((nHour+11)%12 + 1, 2);
      case '%a': return aDays[nDay].slice(0,3);
      case '%A': return aDays[nDay];
      case '%b': return aMonths[nMonth].slice(0,3);
      case '%B': return aMonths[nMonth];
      case '%c': return date.toUTCString();
      case '%C': return Math.floor(nYear/100);
      case '%e': return nDate;
      case '%F': return date.toISOString().slice(0,10);
      case '%G': return getThursday().getFullYear();
      case '%g': return ('' + getThursday().getFullYear()).slice(2);
      case '%H': return zeroPad(nHour, 2);
      case '%j': return zeroPad(aDayCount[nMonth] + nDate + ((nMonth>1 && isLeapYear()) ? 1 : 0), 3);
      case '%k': return '' + nHour;
      case '%P': return (nHour<12) ? 'am' : 'pm';
      case '%s': return Math.round(date.getTime()/1000);
      case '%u': return nDay || 7;
      case '%V': 
              var target = getThursday(),
                n1stThu = target.valueOf();
              target.setMonth(0, 1);
              var nJan1 = target.getDay();
              if (nJan1!==4) target.setMonth(0, 1 + ((4-nJan1)+7)%7);
              return zeroPad(1 + Math.ceil((n1stThu-target)/604800000), 2);
      case '%w': return '' + nDay;
      case '%x': return date.toLocaleDateString();
      case '%X': return date.toLocaleTimeString();
      case '%y': return ('' + nYear).slice(2);
      case '%z': return date.toTimeString().replace(/.+GMT([+-]\d+).+/, '$1');
      case '%Z': return date.toTimeString().replace(/.+\((.+?)\)$/, '$1');
    };
    return sMatch;
  });
}; 
