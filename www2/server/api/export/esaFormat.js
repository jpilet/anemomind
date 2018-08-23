const strftime = require('./strftime');
const format = require('./format');

//// ESA log rendering

function renderAngle(date, val) {
  return (!isFinite(parseFloat(val)) ? '0' : format.number(val, 0));
}

function renderSpeed(date, val) {
  return (val == undefined ? '0.00' : format.number(val, 2));
}

function renderGeoAngle(val, pos, neg) {
  // format is: 4540.8154N
  let abs = Math.abs(parseFloat(val));
  let deg = Math.floor(abs);
  let min = (abs - deg) * 60.0;
  return deg.toFixed(0) + min.toFixed(4) + (val >= 0 ? pos : neg);
}

//// Declaration of all the columns to be exported for the ESA log file format
var esaColumns = [{
  esaName: 'Date',
  render: (date, val) => strftime("%d/%m/%y", date)
}, {
  esaName: 'Time',
  render: (date, val) => strftime("%H:%M:%S", date)
}, {
  esaName: 'Ts',
  render: (date, val) => (date.getTime() / 1000).toFixed(0)
}, {
  esaName: 'Boatspeed',
  fromName: 'gpsSpeed',
  render: renderSpeed
}, {
  esaName: 'AW_angle',
  fromName: 'awa',
  render: renderAngle
}, {
  esaName: 'AW_speed',
  fromName: 'aws',
  render: renderSpeed
}, {
  esaName: 'Heading',
  fromName: 'magHeading',
  render: renderAngle
}, {
  esaName: 'TW_angle',
  fromName: 'twa',
  render: renderAngle
}, {
  esaName: 'TW_speed',
  fromName: 'tws',
  render: renderSpeed
}, {
  esaName: 'TW_Dir',
  fromName: 'twdir',
  render: renderAngle
}, {
  esaName: 'Ext_SOG',
  fromName: 'gpsSpeed',
  render: renderSpeed
}, {
  esaName: 'Ext_COG',
  fromName: 'gpsBearing',
  render: renderAngle
}, {
  esaName: 'Latitudine',
  fromName: 'latitude',
  render: (date, val) => renderGeoAngle(val, 'N', 'S')
}, {
  esaName: 'Longitudine',
  fromName: 'longitude',
  render: (date, val) => renderGeoAngle(val, 'E', 'W')
}, {
  esaName: 'BS_target',
  render: () => '0.00'
}, {
  esaName: 'TWA_target',
  render: () => '0'
}, {
  esaName: 'BS_polar',
  render: () => '0.00'
}, {
  esaName: 'Type_tgt',
  render: () => 'E'
}, {
  esaName: 'LeewayAng',
  render: () => '0.00'
}, {
  esaName: 'LeewayMod',
  render: () => '0.00'
}, {
  esaName: 'SET',
  render: () => '0'
}, {
  esaName: 'DRIFT',
  render: () => '0.00'
}];

function sendEsaHeader(res, columns) {
  res.write('----------------------------- exported_from_anemolab.esa.log --------------------------\n'
  + 'Date	Time	Ts	Boatspeed	AW_angle	AW_speed	Heading	TW_angle	TW_speed	TW_Dir	Ext_SOG	Ext_COG	Latitudine	Longitudine	BS_target	TWA_target	BS_polar	Type_tgt	LeewayAng	LeewayMod	SET	DRIFT\n');
}

// Astra loader does not like holes in the file.
// We fill holes with a value either a bit before or a bit after.
// If none is available, we put 0.
function fillHoles(table, times, col) {
  const findValue = (t) => {
    const maxDist = Math.max(t, times.length - t);
    for (let dist = 1; dist < maxDist; ++dist) {
      if (((t - dist) >= 0)
          && table[times[t - dist]][col] != undefined) {
        return table[times[t - dist]][col];
      }
      if (((t + dist) < times.length)
          && table[times[t + dist]][col] != undefined) {
        return table[times[t + dist]][col];
      }
    }
    return undefined;
  };

  for (let t = 0; t < times.length; t++) {
    let row = table[times[t]];
    if (!isFinite(parseFloat(row[col]))) {
      let newVal = findValue(t);
      if (newVal != undefined) {
        row[col] = newVal;
      } else {
        console.log('Failed to find a value for col ', col, ' at time ', t);
      }
    }
  }
}

function sendEsaChunk(res, columns, table, columnType, columnNames) {
  const times = Object.keys(table);
  times.sort(function(a, b) { return parseInt(a) - parseInt(b); });

  const esaToRow = [];
  for (let i = 0; i < esaColumns.length; ++i) {
    let esaCol = esaColumns[i];
    if (esaCol.fromName) {
      const index = columnNames.indexOf(esaCol.fromName);
      if (index >= 0) {
        esaToRow[i] = index;
        fillHoles(table, times, index);
      } else {
        console.log('Can\'t find column ' + esaCol.fromName + '/' + esaCol.esaName);
      }
    }
  }

  const mandatory =
    ['latitude', 'longitude'].map((x) => columnNames.indexOf(x));

  if (mandatory.indexOf(-1) != -1) {
    console.warn("No GPS column, can't to export ESA file!");
    res.send(500);
    return;
  }

  for (let time_index = 0 ; time_index < times.length; ++ time_index) {
    const t = times[time_index];
    const rowDate = new Date(parseInt(t) * 1000);
    const tableRow = table[t];

    let ok = true;
    for (let m of mandatory) {
      if (!isFinite(parseFloat(tableRow[m]))) {
        ok = false;
      }
    }

    if (!ok) {
      continue;
    }
        
    for (let i = 0; i < esaColumns.length; ++i) {
      const val = (esaToRow[i] ? tableRow[esaToRow[i]] : undefined);
      if (val == undefined && esaToRow[i] != undefined) {
        console.log('Warning, missing value for col ', i);
      }
      const separator = (i == (esaColumns.length -1) ? '\n' : '\t');
      const str = '' + esaColumns[i].render(rowDate, val);
      res.write(str + separator);
    }
  }
}

// Finds the most suitable source for a given channel.
//
// Example usage:
//
//  ChartSource.findById(boat, (err, chartSources) => {
//    if (err) {
//      // handle error
//      return;
//    }
//    const gpsSpeedSource = bestSourceForChannel("gpsSpeed", chartSources);
//    const latitudeSource = bestSourceForChannel("latitude", chartSources);
//    const longitudeSource = bestSourceForChannel("longitude", chartSources);
//  });
function bestSourceForChannel(channel, chartSources) {
  const sources = Object.keys(chartSources.channels[channel]);
  let bestPrio;
  let bestSource;
  for (let s of sources) {
    let prio = chartSources.channels[channel][s].priority
      + (s.match(/mix/) ? .1 : 0);
    if (bestPrio == undefined || prio > bestPrio) {
      bestPrio = prio;
      bestSource = s;
    }
  }
  return bestSource;
}

function columnString(sourceType, chartSources) {
  for (let c of esaColumns) {
    if (c.fromName && c.fromName == sourceType.type
        && sourceType.source == bestSourceForChannel(c.fromName, chartSources)) {
      return c.fromName;
    }
  }
  return undefined;
}

module.exports = {
  contentType: 'text/plain',
  sendHeader: sendEsaHeader,
  sendChunk: sendEsaChunk,
  fileExtension: ".log",
  columnString: columnString
};
