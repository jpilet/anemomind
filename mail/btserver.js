var mb = require('./mail.sqlite.js');
var bigint = require('./bigint.js');
var bleno = require('bleno');
var util = require('util');
var ms = require('./mailservice.js');
var c = require('./rpccodes.js');
var Q = require('q');


var name = bigint.make(119);

function makeService(cb) {
    console.log('Make a mailbox with name ' + name);
    mb.makeMailbox(
	':memory:',
	name,
	function(err, box) {
	    if (err) {
		console.log('Failed to create a mailbox.');
	    }
	    console.log('A mailbox with name ' + box.mailboxName + ' was successfully created');
	    cb(err, new MailService(box));
	}
    );
}

// mailService.
var mailServicePromise = Q.nfcall(makeService);


bleno.on('stateChange', function(state) {
    console.log('stateChange');
    if (state === 'poweredOn') {
	console.log('powered on.');
	mailServicePromise.then(function(mailService) {
	    console.log('Start advertising...');
	    bleno.startAdvertising(name, [mailService.uuid], function(err) {
		if (err) {
		    console.log(err);
		}
	    });
	});
    }
    else {
	bleno.stopAdvertising();
    }
});

bleno.on('advertisingStart', function(err) {
    console.log('Adv start');
    if (!err) {
	console.log('advertising...');

	mailServicePromise.then(function(mailService) {
	    console.log('Set the services');
	    bleno.setServices([
	        mailService
	    ]);
	});
    } else {
	console.log('Error: %j', err);
    }
});



// argHandler(self, args, cb(err, result))
function makeRPCHandler(call, argHandler) {
    return function(data, offset, withoutResponse, cb) {
	console.log('RPC handler called for ' + call.name);
	var self = this;
	if (offset) {
	    cb(this.RESULT_ATTR_NOT_LONG);
	} else {
	    var args = call.args.unwrap(data);
	    argHandler(
		self, args, function(err, result) {
		    if (call.result.wrap == undefined) {
			var data = new Buffer(0);
			self.updateValueCallback(data);
		    } else {
			var data = call.result.wrap(result);
			self.updateValueCallback(data);			
		    }
		    cb(this.RESULT_SUCCESS);
		}
	    );
	}
    };
}



function SetForeignDiaryNumber(mailbox) {
    this.mailbox = mailbox;
    bleno.Characteristic.call(
	this, {
	    uuid: '13333333333333333333333333330001',
	    properties: ['write', 'notify'],
	    descriptors: [
		new bleno.Descriptor(
		    {
			uuid: '2901',
			value: 'Set a foreign diary number and notify when done.'
		    }
		)
	    ]
	}
    );
    console.log('Created characteristic.');
}
util.inherits(SetForeignDiaryNumber, bleno.Characteristic);
SetForeignDiaryNumber.prototype.onWriteRequest =
    makeRPCHandler(
	c.setForeignDiaryNumber,
	function(self, args, cb) {
	    self.mailbox.setForeignDiaryNumber(
		args.mailboxName, args.DiaryNumber, cb
	    );
	}
    );



function GetFirstPacketStartingFrom(mailbox) {
    this.mailbox = mailbox;
    bleno.Characteristic.call(
	this, {
	    uuid: '13333333333333333333333333330002',
	    properties: ['write', 'notify'],
	    descriptors: [
		new bleno.Descriptor(
		    {
			uuid: '2901',
			value: 'Get the first packet starting from a diary number'
		    }
		)
	    ]
	}
    );
    console.log('Created characteristic.');
}
util.inherits(GetFirstPacketStartingFrom, bleno.Characteristic);
GetFirstPacketStartingFrom.prototype.onWriteRequest =
    makeRPCHandler(
	c.getFirstPacketStartingFrom,
	function(self, args, cb) {
	    self.mailbox.getFirstPacketStartingFrom(
		args.mailboxName, args.DiaryNumber, args.lightWeight, cb
	    );
	}
    );


function HandleIncomingPacket(mailbox) {
    this.mailbox = mailbox;
    bleno.Characteristic.call(
	this, {
	    uuid: '13333333333333333333333333330003',
	    properties: ['write', 'notify'],
	    descriptors: [
		new bleno.Descriptor(
		    {
			uuid: '2901',
			value: 'Handle an incoming packet'
		    }
		)
	    ]
	}
    );
    console.log('Created characteristic.');
}
util.inherits(HandleIncomingPacket, bleno.Characteristic);
HandleIncomingPacket.prototype.onWriteRequest =
    makeRPCHandler(
	c.handleIncomingPacket,
	function(self, args, cb) {
	    self.mailbox.handleIncomingPacket(
		args, cb
	    );
	}
    );


function IsAdmissible(mailbox) {
    this.mailbox = mailbox;
    bleno.Characteristic.call(
	this, {
	    uuid: '13333333333333333333333333330004',
	    properties: ['write', 'notify'],
	    descriptors: [
		new bleno.Descriptor(
		    {
			uuid: '2901',
			value: 'Check if it is an admissible packet'
		    }
		)
	    ]
	}
    );
    console.log('Created characteristic.');
}
util.inherits(IsAdmissible, bleno.Characteristic);
IsAdmissible.prototype.onWriteRequest =
    makeRPCHandler(
	c.isAdmissible,
	function(self, args, cb) {
	    self.mailbox.isAdmissible(
		args, cb
	    );
	}
    );


function GetForeignDiaryNumber(mailbox) {
    this.mailbox = mailbox;
    bleno.Characteristic.call(
	this, {
	    uuid: '13333333333333333333333333330005',
	    properties: ['write', 'notify'],
	    descriptors: [
		new bleno.Descriptor(
		    {
			uuid: '2901',
			value: 'Get a foreign diary number and notify when done.'
		    }
		)
	    ]
	}
    );
    console.log('Created characteristic.');
}
util.inherits(GetForeignDiaryNumber, bleno.Characteristic);
GetForeignDiaryNumber.prototype.onWriteRequest =
    makeRPCHandler(
	c.getForeignDiaryNumber,
	function(self, args, cb) {
	    self.mailbox.getForeignDiaryNumber(
		args.mailboxName, cb
	    );
	}
    );

function MailboxName(mailbox) {
    this.mailbox = mailbox;
    bleno.Characteristic.call(
	this, {
	    uuid: '13333333333333333333333333330006',
	    properties: ['write', 'notify'],
	    descriptors: [
		new bleno.Descriptor(
		    {
			uuid: '2901',
			value: 'Get the mailbox name'
		    }
		)
	    ]
	}
    );
    console.log('Created characteristic.');
}
util.inherits(MailboxName, bleno.Characteristic);
MailboxName.prototype.onWriteRequest =
    makeRPCHandler(
	c.getForeignDiaryNumber,
	function(self, args, cb) {
	    // ignore the args: we just want to the the name of this mailbox
	    cb(undefined, self.mailbox.mailboxName);
	}
    );





function MailService(mailbox) {
    bleno.PrimaryService.call(
	this,
	{
	    uuid: '13333333333333333333333333333337',
	    characteristics: [
		new SetForeignDiaryNumber(mailbox),
		new GetFirstPacketStartingFrom(mailbox),
		new HandleIncomingPacket(mailbox),
		new IsAdmissible(mailbox),
		new GetForeignDiaryNumber(mailbox),
		new MailboxName(mailbox)
	    ]
	}
    );
}

util.inherits(MailService, bleno.PrimaryService);





function runIt(err) {
    if (err) {
	console.log('Error creating a mailbox: %j', err);
    }
    


    console.log('Run the mail service!!!');
    

    console.log('Registered advertisingStart');

}
