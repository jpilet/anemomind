const request = require('request-promise-native');
const stream = require('stream');
const fs = require('fs');

const config = require('../../config/environment');
const esapolar = require('../files/esapolar'); 

const credentials = {
  username:"anemomind",
  password:"eexagu$vuagei4Ee"
};

let token;

async function authenticate(credentials) {
  const reply = await request({
          uri: 'https://esa-log-analyzer.astrayacht.net/api/v1/auth',
          method: 'POST',
          body: credentials,
          headers: {
            'Content-Type': 'application/json'
          },
          json: true
  })
  if (reply.error) {
    throw new Error('authentication error: ' + reply.error
                    + (reply.description ? ' (' +reply.description + ')' : ''));
  }
  if (!reply.access_token) {
    throw new Error('authentication: bad reply from server.');
  }
  return reply.access_token;
}

async function getToken() {
  if (!token) {
    token = await authenticate(credentials);
    setTimeout(() => { token = undefined; }, 20 * 60 * 1000);
  }
  return token;
}

function sessionBuffer(sessionUrl) {
  return request(sessionUrl);
}

function sendProcessQuery(yardPolar, logStream, token) {
  /* Example:
   curl https://esa-log-analyzer.astrayacht.net/api/v1/analyze \
      --form instruments=1 \
      --form polar_type=regatta \
      --form 'yard_polar=@30feet.log' \
      --form 'log=@all.esalog' \
      -H 'Authorization: JWT eyJ0eXAi******m_E'
  */
  const options = {
    uri: 'https://esa-log-analyzer.astrayacht.net/api/v1/analyze',
    formData: {
      instruments_type: 1,
      polar_type: 'regatta',
      yard_polar: {
        value: fs.createReadStream(yardPolar),
        options: {
          filename: 'yard_polar.log',
          contentType: 'application/octet-stream'
        }
      },
      log: {
        value: logStream,
        options: {
          filename: 'session.ESA',
          contentType: 'application/octet-stream'
        }
      }
    },
    headers: {
      Authorization: 'JWT ' + token
    }
  };

  return request.post(options);
}

function encodeTime(time) {
  return time.getUTCFullYear() + '-'
    + zeroPad2(time.getUTCMonth() + 1) + '-'
    + zeroPad2(time.getUTCDate())
    + 'T' + zeroPad2(time.getUTCHours())
    + ':' + zeroPad2(time.getUTCMinutes())
    + ':' + zeroPad2(time.getUTCSeconds());
}

async function sendAnalyzeQuery(params) {
  const sessionUrl = 
    'http://localhost:' + config.port + '/api/export/' + params.boatId + '/'
    + encodeTime(params.start) + encodeTime(params.end) + '.esa.log'
    + '?access_token=' + params.token;


  const token = await getToken();

  if (!params.yardPolar) {
    params.yardPolar = __dirname + '/empty_yard_polar.esa';
  }

  const buffer = await sessionBuffer(sessionUrl);

  const result = await sendProcessQuery(params.yardPolar,
                                        buffer,
                                        token);

  return result;
}

function downloadEsaResults(perfStat) {
  return new Promise(async (resolve, reject) => {
    perfStat.status = 'processing';
    perfStat.lastStateChange = new Date();
    perfStat.save();

    try {
      if (!perfStat.analyzeResult
          || !perfStat.analyzeResult.results
          || !perfStat.analyzeResult.results.esa) {
        throw new Error("Missing ESA results");
      }

      const token = await getToken();
      const files = [ "polar", "esa", "stats" ];
      const promises = { };
      for (let f of files) {
        const url = perfStat.analyzeResult.results[f];
        if (!url) {
          continue;
        }
        let file = f; // ensure correct closure capture
        promises[f] = request({
              uri: url,
              method: 'GET',
              headers: {
                'Content-Type': 'application/json',
                Authorization: 'JWT ' + token
              },
              json: true
        }).then((content) => {
          return new Promise((resolve, reject) => {
            const filename = '/tmp/' + perfStat._id + '_' + file;
            fs.writeFile(filename, content, (err) => {
              if (err) {
                console.warn(filename, ": ", err);
                reject(err);
              }
              else {
                console.log('Downloaded ', filename);
                resolve(filename);
              }
            });
          });
        });
      }

      const parsedEsaPolar = await esapolar.readEsaPolar(await promises.esa);

      console.warn('parsedEsaPolar:', parsedEsaPolar);
      perfStat.esaDataRegime = parsedEsaPolar.regime;
      perfStat.esaVmgPoints = parsedEsaPolar.vmgPoints;
      perfStat.polar = parsedEsaPolar.polare;

      perfStat.status = 'ready';
      perfStat.error = null;
      perfStat.lastStateChange = new Date();
      perfStat.save();

      resolve(perfStat);
    } catch (err) {
      console.log("CATCHED: ", err);
      perfStat.error = err.message;
      perfStat.status = 'failed';
      perfStat.lastStateChange = new Date();
      perfStat.save();
      console.warn(err);
      reject(err);
    }
  });
}

module.exports.sendAnalyzeQuery = sendAnalyzeQuery;
module.exports.downloadEsaResults = downloadEsaResults;

