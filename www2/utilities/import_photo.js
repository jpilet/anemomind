const exif = require('exif');
const argv = require('minimist')(process.argv.slice(2));
const Q = require('q');
const mongoose = require('mongoose');
const sharp = require('sharp');
const crypto = require('crypto');
const mkdirp = require('mkdirp');
const fs = require('fs');

process.env.NODE_ENV = process.env.NODE_ENV  || 'development';

const config = require('../server/config/environment/index.js');
const Boat = require('../server/api/boat/boat.model.js');
const Event = require('../server/api/event/event.model.js');


function checkBoatInDb(boatid) {
  const deferred = Q.defer();

  Boat.findById(boatid, deferred.makeNodeResolver());
  return deferred.promise;
}

function parseDate(dateString) {
  return new Date(dateString.replace(/(\d{4}):(\d{2}):(\d{2})/, '$1-$2-$3'));
}

function extractDateFromExifData(exifData) {
  if (!exifData || !Array.isArray(exifData) || exifData.length < 1) {
    throw new Error("No exif data");
  }

  if (exifData[0].gps && exifData[0].GPSDateStamp) {
    const gps = exifData[0].gps;
    const ymd = gps.GPSDateStamp.split(':');
    const hms = gps.GPSTimeStamp;
    console.log(gps.GPSDateStamp + gps.GPSTimeStamp);
    const array = ymd.map(function(x) { return parseInt(x); }).concat(hms);
    array[1] -= 1; // months start at 0, not 1.
    return new Date(Date.UTC.apply(null, array));
  }

  if (exifData[0].exif) {
    for (let key of ['CreateDate', 'DateTimeOriginal']) {
      if (exifData[0].exif[key]) {
        try {
          return parseDate(exifData[0].exif[key]);
        } catch(err) {
          console.warn('Failed to parse date: ', exifData[0].exif[key]);
        }
      }
    }
  }

  throw new Error("Dont known how to extract date from exif data");
}

function getExifAsync(path) {
  const deferred = Q.defer();
  try {
    new exif.ExifImage({ image : path }, deferred.makeNodeResolver());
  } catch(err) {
    deferred.reject(err);
  }
  return deferred.promise;
}

function showTime(path) {
  return getExifAsync(path)
    .then(extractDateFromExifData)
    .then((date) => {
      console.log(path, ': ', date);
    })
    .catch((err) => {
      console.warn(path, ': ', err);
    });
}

function importPhoto(path, boatid, dstpath) {
  console.log('importing: ' + path);

  const image = sharp(path)
  const path_promise =
  image
  .metadata()
  .then(function(metadata) {
    var w,h;
    if (metadata.orientation) {
      h = 1600;
    } else {
      w = 1600;
    }
    return image
      .resize(w, h)
      .rotate()
      .jpeg()
      .toBuffer();
  })
  .then(function(data) {
    const basename = crypto.createHash('md5').update(data).digest("hex") + '.jpg';
    const path = dstpath + '/' + basename;
    console.log('writing resize jpg to: ' + path);

    const deferred = Q.defer();

    fs.writeFile(path, data, function(err) {
      if (err) {
        deferred.reject(err);
      } else {
        deferred.resolve(basename);
      }
    });
    
    return deferred.promise;
  }).catch(function (err) {
    console.warn(err);
  });

  const date_promise = getExifAsync(path).then(extractDateFromExifData);

  return Q.all([path_promise, date_promise]).spread(function(filename, date) {
    console.log(filename + ': ' + date);
    const deferred = Q.defer();
    try {
      const event = new Event({
        boat: mongoose.Types.ObjectId(boatid),
        photo: filename,
        when: date
      });

      event.save(function(err) {
        if (err) {
          deferred.reject(new Error(err));
        } else {
          deferred.resolve(event);
        }
      });
    } catch (err) {
      deferred.reject(err);
    }
    return deferred.promise;
  })
  .then(function(event) {
    console.log('Event for ' + path + ': ' + event.id);
    return event;
  }).catch(function (err) {
    console.warn(err);
  });
}

function main(argv) {
  if (!argv.b && !argv.t) {
    console.warn('Usage: [-t | -b boatid] <pic.jpg> [<pic.jpg> ...]');
    process.exit(1);
  }

  if (argv.t) {
    for (i in argv._) {
      showTime(argv._[i]);
    }
    return;
  }

  mongoose.connect(config.mongo.uri, config.mongo.options);

  const boatid = argv.b;
  const dstpath = config.uploadDir +'/photos/' + boatid;

  checkBoatInDb(boatid)
  .then(function(boat) {
    return Q.nfcall(mkdirp, dstpath);
  })
  .then(function() {
    const promises = [];
    for (i in argv._) {
      promises.push(importPhoto(argv._[i], boatid, dstpath));
    }
    return Q.all(promises);
  })
  .then(function() {
    mongoose.disconnect();
  })
  .catch(function(err) {
    console.warn(err);
    process.exit(2);
  });
}

main(argv);
