'use strict';

const { execFile } = require('child_process');
const backup = require('../../components/backup');
const esaPolar = require('./esapolar');
const LogFile = require('./logfile.model');
const mongoose = require('mongoose');
const util = require('util');


var multer = require('multer');
var fs = require('fs');
var config = require('../../config/environment');
var mkdirp = require('mkdirp');

const fstat = util.promisify(fs.stat);


// Imports the Google Cloud client library
const { Storage } = require('@google-cloud/storage');

var uploadPath = fs.realpathSync(config.uploadDir) + '/anemologs/boat';
console.log('Uploading log files to: ' + uploadPath);

function isFileNameOk(filename) {
  if (!filename || typeof (filename) != 'string') {
    return false;
  }

  var regexp = /^[a-z0-9_-][a-z0-9_., -]+$/i;
  return !!filename.match(regexp);
}

function relativeFileDir(req) {
  if (req.params.boatId && req.params.boatId.match(/^[0-9a-zA-Z_-]+$/)) {
    return req.params.boatId + '/files';
  }
  return undefined;
}

function fileDir(req) {
  const relative = relativeFileDir(req);
  if (relative) {
    // There must be no '/' between uploadPath and boatid, because the folder
    // name looks like: /path/boat12345/files for boat id 12345. Adding a slash
    // would lead to /path/boat/12345/files instead.
    return uploadPath + relative;
  }
  return undefined;
}

function fileName(req) {
  var file = req.params.file;
  if (!isFileNameOk(file)) {
    return undefined;
  }
  return file;
}

function getDetailsForFiles(dir, files) {
  const regex = /ESA$/;
  let esaFiles = files.filter((f) => f.match(regex))
    .map((f) => {
      return { name: f, type: 'ESA Polar' };
    });

  return new Promise((resolve, reject) => {
    const logfiles = files.filter((f) => !f.match(regex));
    if (logfiles.length == 0) {
      return resolve(esaFiles);
    } else {
      execFile(config.tryLoadBin, (dir ? ['-C', dir] : []).concat(logfiles),
        (error, stdout, stderr) => {
          if (stderr) {
            console.log(config.tryLoadBin, ': ', stderr);
          }
          if (error) {
            reject(error);
          }
          try {
            resolve(JSON.parse(stdout).concat(esaFiles));
          } catch (err) {
            console.warn(err);
            resolve(logfiles.map((f) => { return { name: f }; }).concat(esaFiles));
          }
        });
    }
  });
}

function addFileCacheEntry(dir, filename, boatId, size, date, user) {
  return new Promise(async (resolve, reject) => {
    try {
      const logFile = {};
      logFile.name = filename;
      logFile.size = size;
      logFile.boat = new mongoose.Types.ObjectId(boatId);
      logFile.uploadedBy = user;
      logFile.uploadDate = date;
      logFile.type = 'unknown';
      logFile.processed = null;

      const details = await getDetailsForFiles(dir, [filename]);
      if (details.length != 1) {
        logFile.error = "internal error";
      } else if (details[0].error) {
        logFile.error = details[0].error;
      } else {
        if (details[0].type) {
          logFile.type = details[0].type;
        }
        if (details[0].data) {
          logFile.data = details[0].data;
          logFile.type = 'log';
        }
        if (details[0].start) {
          logFile.start = new Date(details[0].start);
          if (details[0].duration_sec) {
            logFile.end = new Date(
              logFile.start.getTime() + 1000 * details[0].duration_sec);
          }
        }
        if (details[0].duration_sec) {
          logFile.duration_sec = details[0].duration_sec;
        }
      }

      await new Promise((resolve) => {
        LogFile.findOneAndUpdate(
          { name: logFile.name, boat: logFile.boat },
          { $set: logFile },
          { upsert: true, returnNewDocument: true },
          function (err, doc) {
            if (err) {
              console.log(err);
              reject(err);
            }
            else
              console.log(doc);
          });
      });


    } catch (err) {
      reject(err);
    }
  });
}

exports.getSingle = async function (req, res, next) {
  const dir = fileDir(req);
  const name = fileName(req);
  if (!dir || !name) {
    res.status(400).send();
    return;
  }

  try {
    const result = await getLogFilesEntry(req.params.boatId, name);
    res.status(200).json(result);
  } catch (err) {
    console.warn(err);
    res.status(404).send();
  }
};

function nodeStyleCallback(resolve, reject) {
  return (err, result) => {
    if (err) {
      return reject(err);
    }
    resolve(result);
  };
}

function getLogFilesEntry(boatId, name) {
  return new Promise((resolve, reject) => {
    if (!name || !boatId) {
      return reject(new Error('Bad arguments.'));
    }
    LogFile.findOne({
      boat: mongoose.Types.ObjectId(boatId),
      name: name,
    }, nodeStyleCallback(resolve, reject));
  });
}
function getLogFilesEntries(boatId) {
  return new Promise((resolve, reject) => {
    LogFile
      .find({ boat: mongoose.Types.ObjectId(boatId) })
      .sort({ uploadDate: -1 })
      .exec(nodeStyleCallback(resolve, reject));
  });
}

exports.listFiles = async function (req, res, next) {
  const boatId = req.params.boatId;

  const promiseLogFiles = getLogFilesEntries(boatId);

  var dir = fileDir(req);
  console.log('fetching file list in ' + dir);
  if (!dir) {
    req.status(400).send();
  } else {
    // let's make sure all the files in 'dir' are in the cache.
    const p = new Promise((resolve, reject) => {
      fs.readdir(dir, function (err, files) {
        if (err) {
          if (err.code == 'ENOENT') {
            // The 'files' folder has not been created. It means there
            // are no file, it is not an error.
            resolve([]);
          } else {
            reject(err);
          }
        } else {
          resolve(files);
        }
      });
    });

    try {
      const logFiles = await promiseLogFiles;
      const files = await p;
      const dict = {};
      for (let log of logFiles) {
        dict[log.name] = log;
      }
      for (let f of files) {
        if (!(f in dict)) {
          console.log(f, ': not in cache. Adding it.');
          const stat = await fstat(dir + '/' + f);
          const newLogFileEntry = await addFileCacheEntry(
            dir, f, boatId, stat.size, new Date(stat.birthtimeMs), req.user);
          logFiles.push(newLogFileEntry);
        }
      }

      res.json(logFiles);
    } catch (err) {
      console.warn(err);
      res.status(500).send(err);
    }
  }
};

// This handler can assume that the user is authentified, it has write access,
// and the upload folder for the boat has been created.
exports.postFile = multer({
  storage: multer.diskStorage({
    destination: function (req, file, cb) {
      var dir = fileDir(req);
      if (dir) {
        mkdirp(dir, () => { cb(null, dir); });
      } else {
        return cb('no boatId', undefined);
      }
    },

    filename: function (req, file, cb) {
      var filename = file.originalname
        .replace(/[^a-z0-9_.-]/gi, '_')
        .replace(/^[.]/, '_'); // initial dot is not allowed

      // For paranoia, let's check the result.
      if (isFileNameOk(filename)) {
        console.log('file handler: accepting file: ' + file.originalname
          + ' as: ' + filename);
        file.newname = filename;
        cb(null, filename);
      } else {
        console.log('file handler: rejecting bad filename: '
          + file.originalname);
        cb('Bad filename', undefined);
      }
    }
  })
}).any();



exports.handleUploadedFile = async (req, res, next) => {
  const dir = fileDir(req);
  if (req.files.length == 0) {
    res.status(400).json({ error: 'please attach at least one file' });
    return;
  }
  if (!dir) {
    console.warn('handleUploadedFile: no dir');
    res.status(500); // should be caught by previous stage
    return;
  }
  if (!req.user) {
    return res.status(401).send();
  }

  const relative = relativeFileDir(req);
  const result = [];
  for (let i of req.files) {
    console.log(dir + '/' + i.newname
      + ' uploaded, size: ' + i.size);
    try {
      const logFile = await addFileCacheEntry(
        dir, i.newname, req.params.boatId, i.size, new Date(), req.user);
    } catch (err) {
      console.warn('Failed to add cache entry for file: ' + dir + '/' + i.newname);
      console.warn(err);
    }
    result.push(i.newname);
  }
  res.status(201).json({ result: 'OK', files: result });
  backup.pushLogFilesToProcessingServer();

  result.forEach((f) => {
    if (f.match(/ESA$/)) {
      esaPolar.readEsaPolar(dir + '/' + f)
        .then((data) => { return esaPolar.uploadEsaPolar(req.params.boatId, data); })
        .catch((err) => {
          console.warn('ESA polar error for file: ', f, ': ', err);
        });
    }
  });
};

exports.delete = async function (req, res, next) {
  const dir = fileDir(req);
  const name = fileName(req);
  if (!dir || !name) {
    res.status(400).send();
    return;
  }

  const err = await new Promise((resolve) => {
    LogFile.deleteOne({
      name: name,
      boat: mongoose.Types.ObjectId(req.params.boatId)
    }, resolve);
  });
  if (err) { console.warn(err); }

  fs.unlink(dir + '/' + name, function (err) {
    if (err) {
      console.warn(err);
      res.status(500).send(err);
    } else {
      res.status(204 /* no content */).send();
    }
  });
};


// uploading file in gcp 
function getGSUrls (filename) {
  return `https://storage.googleapis.com/boat_logs/${filename}`;
}


exports.multerGcp = multer({
 storage: multer.MemoryStorage
}).any();

exports.fileToGcp = async (req, res, next) =>  {
   // Creates a client
  const storage = new Storage({
    projectId: 'anemomind',
    keyFilename: '/anemomind/www2/anemomind-9b757e3fbacb.json'
  });
  
  const bucket = storage.bucket('boat_logs');
  
// boat directory will store all uploaded files
// of the respective boat in boat_logs bucket.
  let boatDir = 'boat' + req.params.boatId + '/';

  for (let f of req.files) {
    try {

      const gcsname = f.originalname;
      const file = bucket.file(boatDir + gcsname);
      
      const stream = file.createWriteStream({
        metadata: {
          contentType: f.mimetype
        },
        resumable: false
      });      
  
      stream.on('error', (err) => {
        f.cloudStorageError = err;
	    console.log("err: ", err);
        next(err);
      });
    
      stream.on('finish', () => {
        f.cloudStorageObject =  boatDir + gcsname;
        file.makePublic().then(() => {
          f.cloudStoragePublicUrl = getGSUrls(boatDir + gcsname);
          next();
        });
      });
      stream.end(f.buffer);
     
    } catch (err) {
      console.warn('Failed to upload file: ' + f.originalname);
      console.warn(err);
    }
    break;
  }
}
