var util = require('util');
var bleno = require('bleno');
var msgpack = require('msgpack');

// To be set using the exported function 'setRpc'
var rpc = {};

function RpcCharacteristic() {
    bleno.Characteristic.call(this, {
	uuid: '13333333333333333333333333330003',
	properties: ['notify', 'write'],
	descriptors: [
	    new bleno.Descriptor({
		uuid: '2901',
		value: 'Responds to RPC calls.'
	    })
	]
    });
}

util.inherits(RpcCharacteristic, bleno.Characteristic);

RpcCharacteristic.prototype.onWriteRequest = function(data, offset, withoutResponse, callback) {
    var self = this;
    if (offset) {
	callback(this.RESULT_ATTR_NOT_LONG);
    } else {
	try {
	    var responded = false;
	    var respond = function(err, value) {
		if (err) {
		    console.log('Called respond function with these args');
		    console.log('  err =   %j', err);
		    console.log('  value = %j', value);
		}
		if (!responded) {
		    self.updateValueCallback(msgpack.pack([err, value]));
		    responded = true;
		}
	    };
	    var call = msgpack.unpack(data);
	    var fun = call.fun;
	    if (!(typeof fun == 'string')) {
		respond('Bad function name, it should be a string but got ' + (typeof fun));
	    } else if (!rpc[fun]) {
		respond('No function registered with name ' + fun);
	    } else {
		var allArgs = call.args.concat([respond]);
		var returnValue = rpc[fun].apply(null, allArgs);
		if (returnValue == undefined) {

		    callback(this.RESULT_SUCCESS);
		    console.log('Successfully evaluated ' + fun);
		    // If we get here, we are successful and should return.		    
		    return;
		    
		} else {
		    // The result should be delivered by 
		    respond('The return value should always be undefined');
		}
	    }
	} catch (e) {
	    console.log('Caught exception: %j', e.message);
	    respond('Failed to process RPC call');
	}

	// If we end up here, we did not return
	// and we are not successful.
	callback(this.RESULT_UNLIKELY_ERROR);
    }
};










///////////////////////////////////////////////////////////////////////////////////////
//// The service
///////////////////////////////////////////////////////////////////////////////////////

function RpcService() {
    bleno.PrimaryService.call(this, {
        uuid: '13333333333333333333333333333337',
        characteristics: [
            new RpcCharacteristic()
        ]
    });
}

util.inherits(RpcService, bleno.PrimaryService);

var name = 'RPC';
var rpcService = new RpcService();

//
// Wait until the BLE radio powers on before attempting to advertise.
// If you don't have a BLE radio, then it will never power on!
//
bleno.on('stateChange', function(state) {
    if (state === 'poweredOn') {
	//
	// We will also advertise the service ID in the advertising packet,
	// so it's easier to find.
	//
	bleno.startAdvertising(name, [rpcService.uuid], function(err) {
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
	    rpcService
	]);
    }
});












/////////////////////////////////////////////////////////////////////////////////
/// EXPORTS
/////////////////////////////////////////////////////////////////////////////////
module.exports.setRpc = function(obj) {
    rpc = obj;
}
