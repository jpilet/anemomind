'use strict';

var express = require('express');
var router = express.Router();
var rpc = require('./rpc.js');
var auth = require('../../auth/auth.service');


/* Available functions to call:

Function name: setForeignDiaryNumber
HTTP-call: GET /setForeignDiaryNumber/:mailboxName/:otherMailbox/:newValue

Function name: getFirstPacketStartingFrom
HTTP-call: GET /getFirstPacketStartingFrom/:mailboxName/:diaryNumber/:lightWeight

Function name: handleIncomingPacket
HTTP-call: POST /handleIncomingPacket/:mailboxName

Function name: isAdmissible
HTTP-call: GET /isAdmissible/:mailboxName/:src/:dst/:seqNumber

Function name: getForeignDiaryNumber
HTTP-call: GET /getForeignDiaryNumber/:mailboxName/:otherMailbox

Function name: getForeignStartNumber
HTTP-call: GET /getForeignStartNumber/:mailboxName/:otherMailbox

Function name: getMailboxName
HTTP-call: GET /getMailboxName/:mailboxName

Function name: reset
HTTP-call: GET /reset/:mailboxName

Function name: sendPacket
HTTP-call: POST /sendPacket/:mailboxName

Function name: getTotalPacketCount
HTTP-call: GET /getTotalPacketCount/:mailboxName
  
*/

rpc.bindMethodHandlers(router, auth.isAuthenticated());
module.exports = router;
