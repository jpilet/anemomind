const PerfStat = require('./perfstats.model');
const mongoose = require('mongoose');
const { promisify } = require('util');
const { downloadEsaResults, sendAnalyzeQuery } = require('./astra-esa');

findOneAsync = promisify(PerfStat.findOne.bind(PerfStat));

function urlFriendlyForm(x) {
  return (x ? x.replace(/[^~a-zA-Z0-9_.,+-]/g, '.').replace(/\.+/g, '.') : '');
}
module.exports.urlFriendlyForm = urlFriendlyForm;

function replyWithMongoResults(res, err, data) {
  if (err) {
    console.warn(new Error(err));
    res.sendStatus(500);
    return;
  }
  if (Array.isArray(data) && data.length == 0) {
    res.sendStatus(404);
    return;
  }
  
  res.json(data).status(200);
}

module.exports.list = (req, res, next) => {
  const boatid = req.params.boatId + '';
  PerfStat.find({ boat: mongoose.Types.ObjectId(boatid) }, (err, data) => {
    if (err) {
      console.warn(new Error(err));
      res.sendStatus(500);
      return;
    }
    if (!data || data.length == 0) {
      res.json([]).status(200);
      return;
    }

    res.json(data.map((obj) => {
      return {
        name: obj.name,
        urlName: obj.urlName || urlFriendlyForm(obj.name),
        type: obj.type,
        status: obj.status || 'ready'
      };
    }))
    .status(200);
  });
};

module.exports.show = (req, res, next) => {
  const boatid = req.params.boatId + '';
  const urlName = req.params.name + '';
  const query = { boat: mongoose.Types.ObjectId(boatid), urlName: urlName };
  console.log('Showing: ', query);
  PerfStat.findOne(query,
                   (err, data) => replyWithMongoResults(res, err, data));
};

function parseDateParam(obj, name, req, res) {
  const reportError = (description) => {
    res.status(400).send('Field: ' + name + ': ' + description);
  };

  let dateStr= obj[name];
  if (!dateStr || !typeof(dateStr) == 'string') {
    reportError('Missing or wrong type');
    return;
  }
  if (dateStr.match(/^[c-z][0-9a-z]{7}$/)) {
    return new Date(parseInt(dateStr, 36));
  }
  const match = dateStr.match(/^20[0-9]{2}-[0-1][0-9]-[0-3][0-9]T[0-2][0-9]:[0-5][0-9]:[0-5][0-9](\.[0-9]*)?(Z)?$/);

  if (match) {
    if (!match[2]) {
      dateStr = dateStr + 'Z';
    }
    return new Date(dateStr);
  }

  reportError('Unrecognized date format. Please use: 2019-12-31T10:41:00'
  + ' or: new Date().getTime().toString(36)');
  return undefined;
}

function overlappingQuery(boatId, start, end) {
  return {
    boat: mongoose.Types.ObjectId(boatId),
    start: {"$lt": end},
    end: {"$gt": start}
  };
}

function findOneOverlapping(boatId, start, end) {
  return promisify((cb) =>
                   PerfStat
                   .findOne(overlappingQuery(boatId, start, end))
                   .sort({created: -1})
                   .exec(cb))();
}

module.exports.findOverlapping = async (req, res, next) => {
  const start = parseRequestDate(req.params,'start', req, res);
  if (!start) {
    return;
  }

  const end = parseRequestDate(req.params, 'end', req, res);
  if (!end) {
    return;
  }

  if (end < start) {
    res.status(400).send('start must come before end.\n'
      + 'start: ' + start + '\nend: ' + end + '\n');
    return;
  }

  try {
    let data = await findOneOverlapping(req.params.boatId, start, end);

    if (data
        && data.type == 'ESA Server'
        && data.status != 'ready'
        && data.analyzeResult
        && data.analyzeResult.results
        && data.analyzeResult.results.esa.match(/^http/)) {
      data = await downloadEsaResults(data);
    }
    replyWithMongoResults(res, undefined, data);
  } catch (err) {
    console.warn(err);
    replyWithMongoResults(res, err, undefined);
  }
};

async function createEsa(req, res, start, end) {
  try {
    let data = await findOneOverlapping(req.params.boatId, start, end);

    if (data) {
      if (data.status == 'ready') {
        return res.status(400).send('Already computed');
      }
        
      if (data.status == 'in-progress') {
        return res.status(400).send('Computation already in progress');
      }
    }

    const name = req.body.name || start +'.esa';
    data = new PerfStat({
        start, end,
        boat: mongoose.Types.ObjectId(req.params.boatId),
        name: name,
        urlName: urlFriendlyForm(name),
        type: 'ESA Server',
        created: new Date(),
        lastStateChange: new Date(),
        status: 'in-progress'
    });

    const resultPromise = sendAnalyzeQuery({
        start,
        end,
        boatId: req.params.boatId,
        token: req.headers.authorization.replace(/^Bearer /, ''),
    });

    await data.validate();
    await data.save();
    let result = await resultPromise;

    if (typeof(result) == 'string') {
      try {
        result = JSON.parse(result);
      } catch(err) {
        console.warn(err);
        data.lastStateChange = new Date();
        data.status = 'failed';
        data.error = 'Error parsing json from ESA api: ' + err.message;
      }
    }
    if (!data.error) {
      if (result.results && result.results.esa) {
        data.status = 'in-progress';
        data.lastStateChange = new Date();
        data.analyzeResult = result;
      } else {
        console.log(result);
        data.lastStateChange = new Date();
        data.status = 'failed';
        data.error = result.error || 'unknown error';
      }
    }
    await data.validate();
    await data.save();

    res.status(201).send(data);
  } catch(err) {
    console.warn(err);
    return res.status(500).send(err.message);
  }
}

function parseRequestDate(obj, key, req, res) {
  let dateStr = obj[key];
  if (!dateStr) {
    return res.status(400).send('Missing field: ' + key);
  }
  dateStr = dateStr + '';

  if (dateStr.substr(-1) != 'Z') {
    dateStr += 'Z';
  }
  
  const date = new Date(dateStr);
  if (isNaN(date)) {
    return res.status(400).send('Invalid date format for ' + key + ': ' + dateStr);
  }

  return date;
}

module.exports.create = (req, res, next) => {
  const start = parseRequestDate(req.body,'start', req, res);
  if (!start) {
    return;
  }

  const end = parseRequestDate(req.body, 'end', req, res);
  if (!end) {
    return;
  }

  if (end < start) {
    res.status(400).send('start must come before end');
    return;
  }

  const hours = 60 * 60 * 1000;
  if ((end.getTime() - start.getTime()) > (24 * hours)) {
    res.status(400).send('Sorry, computation is limited to 24 hours');
    return;
  }

  createEsa(req, res, start, end);
};


