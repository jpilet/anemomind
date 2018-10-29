const mongoose = require('mongoose');
const readline = require('readline');
const fs = require('fs');
const path = require('path');
const PerfStatSchema = require('../perfstats/perfstats.model');

function readEsaPolar(file) {
  return new Promise((resolve, reject) => {
    const stream = fs.createReadStream(file);
    stream.on('error', reject);

    const rl = readline.createInterface({
      input: stream,
      crlfDelay: Infinity
    });

    let state = 'search_header';
    let result = {
      filename: path.basename(file),
      regime: [],
      poligono: [],
      polare: [],
      vmgPoints: []
    };

    const parseError = (reason) => {
      state = 'error';
      stream.close();
      rl.close();
      reject(new Error(file + ': ESA Polar parse error: ' + reason));
    };

    rl.on('line', (line) => {
      if (line.match(/^----+$/)) {
        // in all states, we allow for a line full of '-'
        return;
      }
      switch(state) {
        case 'search_header': {
          const m = line.match(/^--+ (polar.*) ---+$/);
          if (m) {
            result.filenameInHeader = m[1];
          }
          state = 'EstremiSX';
          break;
        }
        case 'EstremiSX':
        case 'EstremiDX': 
        case 'EstremiUP': 
        case 'EstremiDOWN': {
          if (line == state) {
            state = state + '_data';
          } else {
            parseError('Expecting ' + state);
          }
          break;
        }
        case 'EstremiSX_data':
        case 'EstremiDX_data': 
        case 'EstremiUP_data': 
        case 'EstremiDOWN_data': {
          const next = {
            'EstremiSX_data': 'EstremiDX',
            'EstremiDX_data': 'EstremiUP',
            'EstremiUP_data': 'EstremiDOWN',
            'EstremiDOWN_data': 'Data Regime'
          };
	  if (line == next[state]) {
	    if (line == 'Data Regime') {
	      state = 'regime';
	    } else {
	      state = next[state] + '_data';
	    }
	  } else {
	    result[state] = line.split(/\s+/)
	      .map((x) => parseFloat(x))
	      .filter(isFinite);
	    if (result[state].length != 3) {
	      parseError('Bad data for ' + state + ': ' + line);
	    }
	    state = next[state];
	  }
          break;
        }
        case 'Data Regime': 
        case 'Data Poligono':
        case 'Data Polare': {
          if (line == state) {
            state = state.substr(5).toLowerCase();
          } else {
            parseError('Expecting "Data Regime", got: ' + line);
          }
          break;
        }
        case 'regime':
        case 'poligono':
        case 'polare': {
          if (line.match(/^Data Poligono$/)) {
            state = 'poligono';
            break;
          }
          if (line.match(/^Data Polare$/)) {
            state = 'polare';
            break;
          }
          if (line.match(/^VMG Points$/)) {
            state = 'vmg points';
            break;
          }
          const triplet = line.split(/\s+/)
            .map((x) => parseFloat(x))
            .filter(isFinite);
          if (triplet.length < 3 || triplet.length > 4) {
            parseError('Expecting a triplet/quarduplet, got: ' + line);
          } else {
            result[state].push({
              tws: triplet[0],
              twa: triplet[1],
              boatSpeed: triplet[2]
            });
          }
          break;
        }
        case 'vmg points': {
          if (line == 'END') {
            state = 'done';
            stream.close();
            rl.close();
            resolve(result);
            break;
          }

          const numbers = line.split(/\s+/)
            .map((x) => parseFloat(x))
            .filter(isFinite);
          if (numbers.length != 9) {
            parseError('Expecting 9 numbers for vmg points. Line: ' + line);
            break;
          }
          result.vmgPoints.push(numbers);
          break;
        }
        case 'done': break;
	default: parseError('internal error in ESA polar parser, at state: ' + state);
	  break;
      } 
    });
  });
}

function uploadEsaPolar(boat, esaPolar) {
  return new Promise((resolve, reject) => {
    try {
      const id = mongoose.Types.ObjectId(boat);

      // uniqueness of (boat, name) tuple is ensured by the index
      PerfStatSchema.create({
        boat: id,
        name: esaPolar.filename,
        type: 'ESA Polar',
        esaDataRegime: esaPolar.regime,
        esaVmgPoints: esaPolar.vmgPoints,
        polar: esaPolar.polare
      }, (err, createdPerfStats) => {
        if (err) {
          reject(err);
        } else {
          resolve(createdPerfStats);
        }
      });
    } catch(err) {
      reject(err);
    }
  });
}

module.exports.readEsaPolar = readEsaPolar;
module.exports.uploadEsaPolar = uploadEsaPolar;
