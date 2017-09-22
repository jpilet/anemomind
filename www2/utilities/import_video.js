const fs = require('fs');
const path = require('path');

// Set default node environment to development
process.env.NODE_ENV = process.env.NODE_ENV || 'development';
const config = require('../server/config/environment');

if (process.argv.length < 4) {
  console.log('Usage: ' + process.argv[0] + ' ' + process.argv[1] + ' <path to gopro json> <boatid>');
  process.exit(1);
}

const jsonPath = process.argv[2];
const boatid = process.argv[3];

const metadata = JSON.parse(fs.readFileSync(jsonPath, 'utf-8'));

const start = new Date(metadata.data[0].utc / 1000);
const end = new Date(metadata.data[metadata.data.length - 1].utc / 1000);

if (!start || !end) {
  console.warn('something went wrong');
  exit(2);
}

var mongoose = require('mongoose');
mongoose.connect(config.mongo.uri, config.mongo.options);

const Event = require('../server/api/event/event.model.js');

const videoKey = path.basename(jsonPath, '.json');
function addVideo() {
  const event = new Event({
    boat: boatid,
    when: start,
    video: videoKey,
    videoEnd: end
  });

  event.save((err, ev) => {
    if (err) {
      console.warn(err);
    } else {
      console.log('video event saved for boat ' + boatid
		  + ' at time: ' + start + ', end: ' + end);
    }
    mongoose.connection.close();
  });
}

Event.findOne({boat: boatid, when: start, video: videoKey}, (err, event) => {
  if (!err && event) {
    console.log('Video already exists.');
    mongoose.connection.close();
  } else {
    addVideo();
  }
});

