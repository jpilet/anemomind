// A simulation of mailbox synchronizations
var mb = require("./mail.sqlite.js");
var async = require("async");
var q = require("q");

var boxnames = ["A", "B", "C"];


function mailboxesCreated(err, mailboxes) {
    console.log("Err = %j", err);
    console.log("Intiated these mailboxes: %j", mailboxes);
}



// Main call to run the demo
async.map(
    boxnames,
    function (boxname, cb) {
	var box = new mb.Mailbox(":memory:", boxname, function(err) {
	    cb(err, box);
	});
    },
    mailboxesCreated
);
