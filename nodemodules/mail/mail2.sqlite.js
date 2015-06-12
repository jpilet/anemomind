var TransactionDatabase = require("sqlite3-transactions").TransactionDatabase;
var sqlite3 = require('sqlite3').verbose();
var pkt = require('./packet.js');
var bigint = require('./bigint.js');
var common = require('./common.js');
var naming = require('./naming.js');

var fullschema = "CREATE TABLE IF NOT EXISTS packets (src TEXT, dst TEXT, \
seqNumber TEXT, label INT, data BLOB, PRIMARY KEY(src, dst, seqNumber)); \
CREATE TABLE IF NOT EXISTS lowerbounds (src TEXT, dst TEXT, lower TEXT, PRIMARY KEY(src, dst));";

function createAllTables(db, cb) {
  db.exec(fullschema, cb);
}

function openDBWithFilename(dbFilename, cb) {
  var db = new TransactionDatabase(
    new sqlite3.Database(
      dbFilename,
      function(err) {
	if (err != undefined) {
	  cb(err);
	}
      }
    )
  );
  createAllTables(db, function(err) {
    if (err) {
      cb(err);
    } else {
      cb(undefined, db);
    }
  });
}





function EndPoint(filename, mailboxName, db) {
  this.db = db;
  this.dbFilename = filename;
  this.mailboxName = mailboxName;
}

function tryMakeEndPoint(filename, mailboxName, cb) {
  if (!(common.isString(filename) && common.isIdentifier(mailboxName))) {
    cb(new Error('Invalid input to tryMakeEndPoint'));
  } else {
    openDBWithFilename(filename, function(err, db) {
      if (err) {
        cb(err);
      } else {
        cb(null, new EndPoint(filename, mailboxName, db));
      }
    });
  }
}

module.exports.EndPoint = EndPoint;
module.exports.tryMakeEndPoint = tryMakeEndPoint;
