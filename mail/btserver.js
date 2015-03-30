var mb = require('./mail.sqlite.js');
var bigint = require('./bigint.js');
var bleno = require('bleno');
var util = require('util');
var ms = require('./mailservice.js');
var c = require('./rpccodes.js');


    // argHandler(self, args, cb(err, result))
    function makeRPCHandler(call, argHandler) {
	return function(data, offset, withoutResponse, cb) {
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



var name = bigint.make(0);

var mailbox = new mb.Mailbox(
    ':memory:',
    name,
    runIt
);

function runIt(err) {
    console.log('Mailbox created');

    var mailService = new ms.MailService(mailbox);

    console.log('Run the mail service!!!');
    bleno.on('stateChange', function(state) {
	if (state === 'poweredOn') {
	    console.log('powered on.');
	    //
	    // We will also advertise the service ID in the advertising packet,
	    // so it's easier to find.
	    //
	    bleno.startAdvertising(name, [mailService.uuid], function(err) {
		if (err) {
		    console.log(err);
		}
	    });
	}
	else {
	    bleno.stopAdvertising();
	}
    });

    bleno.on('advertisingStart', function(err) {
	if (!err) {
	    console.log('advertising...');
	    //
	    // Once we are advertising, it's time to set up our services,
	    // along with our characteristics.
	    //
	    bleno.setServices([
		mailService
	    ]);
	} else {
	    console.log('Error: %j', err);
	}
    });

}
