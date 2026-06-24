const strftime = require('./strftime');
const format = require('./format');

function columnString(sourceType) {
  return sourceType.type + ' - ' + sourceType.source;
}

function csvEscape(s) {
  if (!s) {
    return '"-"';
  }
  return '"' + s.replace(/"/g, '""').replace(/,/g,';') + '"';
}

function sendCsvHeader(res, columns) {
  var row = [ "DATE/TIME(UTC)", "OADate" ].concat(columns.map(columnString)).map(csvEscape);
  res.write(row.join(',') + '\n');
}

// From https://stackoverflow.com/questions/15549823/oadate-to-milliseconds-timestamp-in-javascript
function dateToOADate(date) {
  let temp = new Date(date);
  const msPerDay = 8.64e7;
  // Set temp to start of day and get whole days between dates,
  const days = Math.round((temp.setHours(0,0,0,0) - new Date(1899, 11, 30)) / 8.64e7);
  // Get decimal part of day, OADate always assumes 24 hours in day
  var partDay = (Math.abs((date - temp) % msPerDay) / 8.64e7);
  return days + partDay;
}

function formatTime(date) {
  // 06/20/2018 4:26:35 PM
  return strftime("\"%m/%d/%Y %l:%M:%S %p\"", date);
  /* The following is the reference implementation, but it is too slow.
  return date.toLocaleString('en-US', {
    year: 'numeric',
    month: '2-digit',
    day: '2-digit',
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit',
    hour12: true,
    timeZone: 'UTC'
  }).replace(/,/, '');
  */
}



function formatColumnEntry(type, entry) {
  if (entry == undefined) {
    return '';
  }

  switch (type) {
    case 'longitude':
    case 'latitude':
      return entry;
    case 'awa':
    case 'twa':
    case 'gpsBearing':
    case 'magHdg':
    case 'twdir':
    case 'vmg':
    case 'targetVmg':
      return format.number(entry, 1);
    case 'aws':
    case 'tws':
    case 'rudderAngle':
    case 'waterSpeed':
    case 'gpsSpeed':
      return format.number(entry, 2);
    default:
      return format.number(entry, 2);
  }
}

function sendCsvChunk(res, columns, table, columnType) {
  var numCols = 2 + columns.length;

  var times = Object.keys(table);
  times.sort(function(a, b) { return parseInt(a) - parseInt(b); });

  for (var time_index = 0 ; time_index < times.length; ++ time_index) {
    var t = times[time_index];

    var rowDate = new Date(parseInt(t) * 1000);

    row = [];
    row[0] = formatTime(rowDate);
    row[1] = dateToOADate(rowDate).toFixed(8);
    var tableRow = table[t];
    for (var i = 0; i < numCols; ++i) {
      row[i + 2] = formatColumnEntry(columnType[i], tableRow[i]); 
    }
    res.write(row.join(', ') + '\n');
  }
}

module.exports = {
  contentType: 'text/csv',
  sendHeader: sendCsvHeader,
  sendChunk: sendCsvChunk,
  fileExtension: ".csv",
  columnString: columnString
};
