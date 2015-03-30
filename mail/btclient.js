var noble = require('noble');
var c = require('./rpccodes.js');
var Q = require('q');
var mb = require('./mail.sqlite.js');
var bigint = require('./bigint.js');
var sync = require('./sync.js');

function makeUuidMap() {
    return {
	// Maps uuids of detected services to their names
	'13333333333333333333333333330001': 'setForeignDiaryNumber',
	'13333333333333333333333333330002': 'getFirstPacketStartingFrom',
	'13333333333333333333333333330003': 'handleIncomingPacket',
	'13333333333333333333333333330004': 'isAdmissible',
	'13333333333333333333333333330005': 'getForeignStartNumber',
	'13333333333333333333333333330006': 'mailboxName',

	// Maps names to characteristics. To be filled in upon detection
	// of a device.
	'setForeignDiaryNumber': null,
	'getFirstPacketStartingFrom': null,
	'handleIncomingPacket': null,
	'isAdmissible': null,
	'getForeignStartNumber': null,
	'mailboxName': null
    };
}

function isComplete(m) {
    for (var key in m) {
	if (m[key] == undefined) {
	    return false;
	}
    }
    return true;
}

var mailServiceUuid = '13333333333333333333333333333337';




function isMapComplete(x) {
    var count = 0;
    for (key in x) {
	if (x[key] == undefined) {
	    return false;
	}
    }
    return true;
}


noble.on('stateChange', function(state) {
  if (state === 'poweredOn') {
    console.log('scanning...');
    noble.startScanning([mailServiceUuid], false);
  }
  else {
    noble.stopScanning();
  }
});


function synchronize(localMailbox, service, cmap) {
    console.log('synchronize');
    makeRpcCall(
	cmap.mailboxName,
	c.mailboxName,
	undefined,
	function(err, lname) {
	    var remoteMailbox = new Mailbox(lname, service, cmap);
	    sync.synchronize(localMailbox, remoteMailbox, function() {
		console.log('Done synchronizing');
	    });
	}
    );
}

function handleService(localMailbox, service) {
    console.log('found service:', service.uuid);
    service.discoverCharacteristics(
	[],
	function(err, characteristics) {
	    var uuidMap = makeUuidMap();
	    characteristics.forEach(
		function(characteristic) {
		    console.log('found characteristic:', characteristic.uuid);
		    var cname = uuidMap[characteristic.uuid];
		    console.log('  which is ' + cname);
		    uuidMap[cname] = characteristic;
		}
	    );
	    if (isComplete(uuidMap)) {
		synchronize(localMailbox, service, uuidMap);
	    }
	    else {
		console.log('missing characteristics: %j', uuidMap);
	    }
	}
    ); // discoverCharacteristics
} // function(service)

var localMailboxPromise = Q.nfcall(mb.makeMailbox, ":memory:", bigint.make(60));

noble.on(
    'discover',
    function(peripheral) {
	console.log('found peripheral:', peripheral.advertisement);
	peripheral.connect(
	    function(err) {
		peripheral.discoverServices(
		    [mailServiceUuid], function(err, services) {
			services.forEach(
			    function (service) {
				console.log('handle service');
				localMailboxPromise.then(
				    function(localMailbox) {
					handleService(localMailbox, service);
				    }
				);
			    }
			); // foreach
		    } // function(err, services)
		);
	    }
	);
    }
);

function makeRawRpcCall(characteristic, codedArgs, cb) {
    characteristic.on(
	'read',
	cb
    );
    characteristic.notify(
	true, function(err) {
	    if (err) {
		console.log('RPC notify error: ', err);
	    }
            characteristic.write(
		codedArgs, false, function(err) {
		    if (err) {
			console.log('RPC write error: ', err);
		    }
		}
	    );
	}
    );
}

function makeRpcCall(characteristic, call, args, cb) {
    makeRawRpcCall(
	characteristic,
	(call.args == undefined? new Buffer(0) : call.args.wrap(args)),
	function(wrappedResult, notified) {
	    console.log('Received wrapped result: %j', wrappedResult);
	    var unwrapped = call.result.unwrap(wrappedResult);
	    console.log('The unwrapped result is: %j', unwrapped);

	    // drop the 'notified' argument, why would that be interesting?
	    cb(undefined, unwrapped);
	}
    );
}

function Mailbox(name, service, cmap) {
    console.log('Create a new mailbox with name ', name);
    this.mailboxName = name;
    this.service = service;
    this.cmap = cmap;
}

Mailbox.prototype.setForeignDiaryNumber = function(otherMailbox, newValue, cb) {
    makeRpcCall(this.cmap.setForeignDiaryNumber,
		c.setForeignDiaryNumber,
		{mailboxName: otherMailbox, diaryNumber: newValue},
		cb);
}

Mailbox.prototype.getFirstPacketStartingFrom =
    function(diaryNumber, lightWeight, cb) {
	console.log('Get first packet starting from:');
	console.log('diaryNumber = ' + diaryNumber);
	console.log('lightWeight = ' + lightWeight);
	makeRpcCall(this.cmap.getFirstPacketStartingFrom,
		   c.getFirstPacketStartingFrom,
		   {diaryNumber: diaryNumber,
		    lightWeight: lightWeight},
		    function(err, value) {
			console.log("Got this packet: %j", value);
			cb(err, value);
		    });
}

Mailbox.prototype.handleIncomingPacket = function(packet, cb) {
    makeRpcCall(this.cmap.handleIncomingPacket, c.handleIncomingPacket,
	       packet, cb);
}

Mailbox.prototype.isAdmissible = function(src, dst, cb) {
    makeRpcCall(this.cmap.isAdmissible, c.isAdmissible, {src: src, dst: dst}, cb);
}

Mailbox.prototype.getForeignStartNumber = function(mailboxName, cb) {
    makeRpcCall(this.cmap.setForeignDiaryNumber,
		c.setForeignDiaryNumber,
		mailboxName,
		cb);
}
