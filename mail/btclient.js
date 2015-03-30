var noble = require('noble');
var c = require('./rpccodes.js');
var Q = require('q');
var mb = require('./mail.sqlite.js');
var bigint = require('./bigint.js');

function makeUuidMap() {
    return {
	// Maps uuids of detected services to their names
	'13333333333333333333333333330001': 'setForeignDiaryNumber',
	'13333333333333333333333333330002': 'getFirstPacketStartingFrom',
	'13333333333333333333333333330003': 'handleIncomingPacket',
	'13333333333333333333333333330004': 'isAdmissible',
	'13333333333333333333333333330005': 'getForeignDiaryNumber',
	'13333333333333333333333333330005': 'mailboxName',

	// Maps names to characteristics. To be filled in upon detection
	// of a device.
	'setForeignDiaryNumber': null,
	'getFirstPacketStartingFrom': null,
	'handleIncomingPacket': null,
	'isAdmissible': null,
	'getForeignDiaryNumber': null,
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
	function(name) {
	    var remoteMailbox = new Mailbox(name, service, cmap);	    
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
	call.args.wrap(args),
	function(wrappedResult, notified) {
	    cb(call.result.unwrap(wrappedResult), notified);
	}
    );
}

function Mailbox(name, service, cmap) {
    console.log('Create a new mailbox with name ', name);
    this.name = name;
    this.service = service;
    this.cmap = cmap;
}

Mailbox.prototype.setForeignDiaryNumber = function(otherMailbox, newValue, cb) {
    makeRpcCall(cmap.setForeignDiaryNumber,
		c.setForeignDiaryNumber,
		{mailboxName: otherMailbox, diaryNumber: newValue},
		cb);
}

Mailbox.prototype.getFirstPacketStartingFrom =
    function(mailboxName, diaryNumber, lightWeight, cb) {
	makeRpcCall(cmap.getFirstPacketStartingFrom,
		   c.getFirstPacketStartingFrom,
		   {mailboxName: mailboxName, diaryNumber: diaryNumber, lightWeight: lightWeight},
		    cb);
}

Mailbox.prototype.handleIncomingPacket = function(packet, cb) {
    makeRpcCall(cmap.handleIncomingPacket, c.handleIncomingPacket,
	       packet, cb);
}

Mailbox.prototype.isAdmissible = function(src, dst, cb) {
    makeRpcCall(cmap.isAdmissible, c.isAdmissible, {src: src, dst: dst}, cb);
}
