var TransactionDatabase = require("sqlite3-transactions").TransactionDatabase;
var sqlite3 = require('sqlite3').verbose();
var pkt = require('./packet.js');
var bigint = require('./bigint.js');
var common = require('./common.js');
var naming = require('./naming.js');

var fullschema = "CREATE TABLE IF NOT EXISTS packets (src TEXT, dst TEXT, \
seqNumber TEXT, label INT, data BLOB, PRIMARY KEY(src, dst, seqNumber)); \
CREATE TABLE IF NOT EXISTS lowerbounds (src TEXT, dst TEXT, lowerbound TEXT, PRIMARY KEY(src, dst));";

function beginTransaction(db, cb) {
  //console.log("BEGIN TRANSACTION");
  //inTransaction = true;
  db.beginTransaction(cb);
}

function commit(T, cb) {
  //console.log("END TRANSACTION");
  //inTransaction = false;
  T.commit(cb);
}

function withTransaction(db, cbTransaction, cbDone) {
  beginTransaction(db, function(err, T) {
    if (err) {
      cbDone(err);
    } else {
      cbTransaction(T, function(err, results) {

        // Called from rollback or commit
        var onFinish = function(e) {
          var totalErr = e || err;
          if (totalErr) {
            cbDone(totalErr);
          } else {
            cbDone(null, results);
          }
        };
        
        if (err) {
          T.rollback(onFinish);
        } else {
          T.commit(onFinish);
        }
      });
    }
  });
}


function createAllTables(db, cb) {
  db.exec(fullschema, cb);
}

function dropTables(db, cb) {
  var names = ['packets', 'lowerbounds'];
  var query = '';
  for (var i = 0; i < names.length; i++) {
    query += 'DROP TABLE IF EXISTS ' + names[i] + ';';
  }
  db.exec(query, cb);
}


function getUniqueSrcDstPairs(db, tableName, cb) {
  db.all('SELECT DISTINCT src,dst FROM ' + tableName + ' ORDER BY src, dst', cb);
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





function EndPoint(filename, name, db) {
  this.db = db;
  this.dbFilename = filename;
  this.name = name;
}

function tryMakeEndPoint(filename, name, cb) {
  if (!(common.isString(filename) && common.isIdentifier(name))) {
    cb(new Error('Invalid input to tryMakeEndPoint'));
  } else {
    openDBWithFilename(filename, function(err, db) {
      if (err) {
        cb(err);
      } else {
        cb(null, new EndPoint(filename, name, db));
      }
    });
  }
}

EndPoint.prototype.reset = function(cb) {
  beginTransaction(this.db, function(err, T) {
    if (err) {
      cb(err);
    } else {
      dropTables(T, function(err) {
	if (err) {
	  cb(err);
	} else {
	  createAllTables(T, function(err) {
	    commit(T, function(err2) {
	      cb(err || err2);
	    });
	  });
	}
      });
    }
  });
}

function tryMakeAndResetEndPoint(filename, name, cb) {
  tryMakeEndPoint(filename, name, function(err, ep) {
    if (err) {
      cb(err);
    } else {
      ep.reset(function(err) {
        if (err) {
          cb(err);
        } else {
          cb(null, ep);
        }
      });
    }
  });
}

function getLowerBoundFromTable(db, src, dst, cb) {
  db.get(
    'SELECT lowerbound FROM lowerbounds WHERE src = ? AND dst = ?',
    src, dst, function(err, row) {
      if (err) {
        cb(err);
      } else {
        if (row) {
          cb(null, row.lowerbound);
        } else {
          cb();
        }
      }
    });
}

EndPoint.prototype.getLowerBound = function(src, dst, cb) {
  withTransaction(
    this.db,
    function(T, cb) {
      getLowerBoundFromTable(T, src, dst, function(err, lowerbound) {
        if (err) {
          cb(err);
        } else if (lowerbound) {
          cb(null, lowerbound);
        } else {
          cb(null, bigint.zero());
        }
      });
    }, cb);
}


module.exports.EndPoint = EndPoint;
module.exports.tryMakeEndPoint = tryMakeEndPoint;
module.exports.tryMakeAndResetEndPoint = tryMakeAndResetEndPoint;
