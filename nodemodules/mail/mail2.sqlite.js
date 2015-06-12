var TransactionDatabase = require("sqlite3-transactions").TransactionDatabase;
var sqlite3 = require('sqlite3').verbose();
var pkt = require('./packet.js');
var bigint = require('./bigint.js');
var common = require('./common.js');
var naming = require('./naming.js');

var fullschema = "CREATE TABLE IF NOT EXISTS packets (src TEXT, dst TEXT, \
seqNumber TEXT, label INT, data BLOB, PRIMARY KEY(src, dst, seqNumber)); \
CREATE TABLE IF NOT EXISTS lowerbounds (src TEXT, dst TEXT, lower TEXT, PRIMARY KEY(src, dst));";
