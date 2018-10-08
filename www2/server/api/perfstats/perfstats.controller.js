const PerfStat = require('./perfstats.model');
const mongoose = require('mongoose');

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
    }

    res.json(data.map((obj) => { return { name: obj.name, type: obj.type }; }))
    .status(200);
  });
};

module.exports.show = (req, res, next) => {
  const boatid = req.params.boatId + '';
  const name = req.params.name + '';
  const query = { boat: mongoose.Types.ObjectId(boatid), name: name };
  console.log('Showing: ', query);
  PerfStat.findOne(query,
                   (err, data) => replyWithMongoResults(res, err, data));
};
