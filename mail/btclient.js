var noble = require('noble');
var c = require('./rpccodes.js');

function makeUuidMap() {
    return {
	'13333333333333333333333333330001': 'setForeignDiaryNumber',
	'13333333333333333333333333330002': 'GetFirstPacketStartingFrom',
	'13333333333333333333333333330003': 'HandleIncomingPacket',
	'13333333333333333333333333330004': 'IsAdmissible',
	'13333333333333333333333333330005': 'GetForeignDiaryNumber'
	'setForeignDiaryNumber': null,
	'getFirstPacketStartingFrom': null,
	'handleIncomingPacket': null,
	'isAdmissible': null,
	'getForeignDiaryNumber': null
    };
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
})

var mailService = null;
var setForeignDiaryNumber = null;
var getFirstPacketStartingFrom = null;
var handleIncomingPacket = null;
var isAdmissible = null;
var getForeignDiaryNumber = null;

function synchronize(localMailbox, service, cmap) {

    // A remote mailbox.
    var remoteMailbox = new Mailbox(service, cmap);

    
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
		    uuidMap[characteristic.uuid] = characteristic;
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

function connectAndSynchronize(localMailbox) {
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
				    handleService(localMailbox, service);
				}
			    ); // foreach
			} // function(err, services)
		    );
		}
	    );
	}
    );
}

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

function Mailbox(service, cmap) {
    this.service = service;
    this.cmap = cmap;
}

Mailbox.prototype

