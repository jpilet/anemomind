var TransactionDatabase = require("sqlite3-transactions").TransactionDatabase;
var sqlite3 = require('sqlite3').verbose();
var pkt = require('./packet.js');
var bigint = require('./bigint.js');
var common = require('./common.js');
var naming = require('./naming.js');

var fullschema = "CREATE TABLE IF NOT EXISTS packets (src TEXT, dst TEXT, \
seqNumber TEXT, label INT, data BLOB, PRIMARY KEY(src, dst, seqNumber)); \
CREATE TABLE IF NOT EXISTS lowerBounds (src TEXT, dst TEXT, lowerBound TEXT, PRIMARY KEY(src, dst));";

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
  var names = ['packets', 'lowerBounds'];
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
    'SELECT lowerBound FROM lowerBounds WHERE src = ? AND dst = ?',
    src, dst, function(err, row) {
      if (err) {
        cb(err);
      } else {
        if (row) {
          cb(null, row.lowerBound);
        } else {
          cb();
        }
      }
    });
}

function getFirstPacketIndex(db, src, dst, cb) {
  db.get(
    'SELECT seqNumber FROM packets WHERE src = ? AND dst = ? ORDER BY seqNumber',
    src, dst,
    function(err, row) {
      if (err) {
        cb(err);
      } else {
        if (row) {
          cb(null, row.seqNumber);
        } else {
          cb();
        }
      }
    });
}

/*

  Try in this order:
  
  1. Read it from the table lowerBounds
  2. Try to retrieve the first packet index
  3. Return 0
   
*/
function getLowerBound(T, src, dst, cb) {
  getLowerBoundFromTable(T, src, dst, function(err, lowerBound) {
    if (err) {
      cb(err);
    } else if (lowerBound) {
      cb(null, lowerBound);
    } else {
      getFirstPacketIndex(T, src, dst, function(err, lowerBound) {
        if (err) {
          cb(err);
        } else if (lowerBound) {
          cb(null, lowerBound);
        } else {
          cb(null, bigint.zero());
        }
      });
    }
  });
}

EndPoint.prototype.getLowerBound = function(src, dst, cb) {
  withTransaction(
    this.db,
    function(T, cb) {
      getLowerBound(T, src, dst, cb);
    }, cb);
}

function getPacketSub(db, src, dst, seqNumber, cb) {
  db.get(
    'SELECT * FROM packets WHERE src = ? AND dst = ? AND seqNumber = ?',
    src, dst, seqNumber, function(err, row) {
      if (err) {
        cb(err);
      } else {
        cb(null, row);
      }
    });
}

EndPoint.prototype.getPacket = function(src, dst, seqNumber, cb) {
  getPacketSub(this.db, src, dst, seqNumber, cb);
}

function getLastPacket(db, src, dst, cb) {
  db.get('SELECT * FROM packets WHERE src = ? AND dst = ? ORDER BY seqNumber DESC',
         src, dst, cb);
}

function getUpperBound(db, src, dst, cb) {
  getLastPacket(db, src, dst, function(err, packet) {
    if (err) {
      cb(err);
    } else if (packet) {
      cb(null, bigint.inc(packet.seqNumber));
    } else {
      getLowerBound(db, src, dst, cb);
    }
  });
}

EndPoint.prototype.getUpperBound = function(src, dst, cb) {
  // First try the lastPacket+1
  // Then the lower bound
  withTransaction(this.db, function(T, cb) {
    getUpperBound(T, src, dst, cb);
  }, cb);
}

EndPoint.prototype.sendPacketAndReturn = function(dst, label, data, cb) {
  var self = this;
  var src = self.name;
  withTransaction(this.db, function(T, cb) {
    getLastPacket(T, src, dst, function(err, packet) {
      var seqNumber = (packet? bigint.inc(packet.seqNumber) : bigint.makeFromTime());
      T.run(
        'INSERT INTO packets VALUES (?, ?, ?, ?, ?)',
        src, dst, seqNumber, label, data, function(err) {
          if (err) {
            cb(err);
          } else {
            cb(null, {
              src: src,
              dst: dst,
              label: label,
              seqNumber: seqNumber,
              data: data
            });
          }
        });
    });
  }, cb);
}

function setLowerBoundInTable(db, src, dst, lowerBound, cb) {
  db.get(
    'SELECT * FROM lowerBounds WHERE src = ? AND dst = ?',
    src, dst, function(err, row) {
      if (err) {
        cb(err);
      } else {
        if (row) {
          db.run(
            'UPDATE lowerBounds SET lowerBound = ? WHERE src = ? AND dst = ?',
            lowerBound, src, dst, cb);
        } else {
          db.run(
            'INSERT INTO lowerBounds VALUES (?, ?, ?)',
            src, dst, lowerBound, cb);
        }
      }
    });
}

function removeObsoletePackets(db, src, dst, lowerBound, cb) {
  db.run(
    'DELETE FROM packets WHERE src = ? AND dst = ? AND seqNumber < ?',
    src, dst, lowerBound, cb);
}

EndPoint.prototype.getTotalPacketCount = function(cb) {
  var query = 'SELECT count(*) FROM packets';
  this.db.get(
    query, function(err, row) {
      if (err == undefined) {
	cb(err, row['count(*)']);
      } else {
	cb(err);
      }
    });
}

function setLowerBound(db, src, dst, lowerBound, cb) {
  getLowerBound(db, src, dst, function(err, currentLowerBound) {
    if (err) {
      cb(err);
    } else {
      if (currentLowerBound < lowerBound) {
        setLowerBoundInTable(db, src, dst, lowerBound, function(err) {
          if (err) {
            cb(err);
          } else {
            removeObsoletePackets(db, src, dst, lowerBound, cb);
          }
        });
      } else {
        cb();
      }
    }
  });
}

EndPoint.prototype.setLowerBound = function(src, dst, lowerBound, cb) {
  withTransaction(this.db, function(T, cb) {
    setLowerBound(T, src, dst, lowerBound, cb);
  }, cb);
}


module.exports.EndPoint = EndPoint;
module.exports.tryMakeEndPoint = tryMakeEndPoint;
module.exports.tryMakeAndResetEndPoint = tryMakeAndResetEndPoint;
