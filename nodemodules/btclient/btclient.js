var noble = require('noble');
var msgpack = require('msgpack');
var Q = require('q');
var assert = require('assert');

// These numbers must be in sync with the bt peripheral code.
var rpcCharacteristicUuid = '13333333333333333333333333330003';
var rpcServiceUuid = '13333333333333333333333333333337';


// The code below just sets up bleno to scan for
// devices.
noble.on('stateChange', function(state) {
    if (state === 'poweredOn') {
	console.log('scanning...');
	noble.startScanning([rpcServiceUuid], false);
    }
    else {
	noble.stopScanning();
    }
})

// RPC manager handles calls to remote functions over bluetooth.
// An instance of this class is exported from this module.
// The RPC manager has a method called 'setCalls' that specifies what
// remote functions can be called. Those functions will be available
// as methods of the RPC manager object.
function RPCManager() {

    // This is a callback that is called when a connection is established.
    this.deferredOnConnect = Q.defer();

    // The characteristic that we expect to get in the future.
    this.deferredCharacteristic = Q.defer();

    // This is the function that should be called for the last callback.
    this.cb = undefined;

    // This one will be resolved when we have obtained a
    // characteristic and initialization has been performed.
    this.deferredInitialized = Q.defer();

    var self = this;

    // Once we obtain the characteristic,
    // we should do some initialization.
    this.deferredCharacteristic.promise.then(
	function(characteristic) {

	    // This makes sure that a callback is called 
	    characteristic.on('read', function(data, isNotification) {
		if (self.cb) {
		    var errOrResult = msgpack.unpack(data);
		    // errOrResult is an array with two elements:
		    //   [success, x]
		    // where success=true iff there was no error,
		    // and x is either the result or the error object.
		    var success = errOrResult[0];
		    var x = errOrResult[1];
		    if (success) {
			self.cb(undefined, x);
		    } else {
			self.cb(x);
		    }
		}
		self.cb = undefined;
	    });

	    // We want to make sure that we have registered 'read'
	    // before we signal that the object is initialized.
	    self.deferredInitialized.resolve(true);

	    // Do something once a connection is established.
	    self.deferredOnConnect.promise.then(
		function(onConnect) {
		    onConnect(undefined, self);
		}
	    );
	}
    );
}

// Used to register a function that is called once
// a connection is established.
RPCManager.prototype.setOnConnect = function(onConnect) {
    assert(typeof onConnect == 'function');
    this.deferredOnConnect.resolve(onConnect);
}

// Call a remote function
RPCManager.prototype.call = function(functionName, args, cb) {
    assert(typeof functionName == 'string');
    assert(typeof cb == 'function');
    
    var self = this;
    self.deferredInitialized.promise.then(
	function(isInitialized) {
	    assert(isInitialized, 'This value only resolves to true');
	    assert(self.cb == undefined, 'Concurrent RPC calls are currently not supported.');

	    // This function will be called
	    // once the server has responded.
	    self.cb = cb;
	    
	    self.deferredCharacteristic.promise.then(
		function(characteristic) {

		    // Copied from the pizza example, not sure what it does.
		    characteristic.notify(
			true,
			function (err) {

			    // Send the RPC call.
			    characteristic.write(
				msgpack.pack(
				    {fun: functionName, args: args}
				),
				false,
				function(err) {
				    if (err) {
					cb(err);
				    }
				}
			    );
			}
		    );   
		}
	    );
	}
    );
}

// Returns a function that, once called, will
// call a remote function over bluetooth.
// makeRpcCall is used by the 'setCalls' method
// of RPCManagaer.
function makeRpcCall(self, functionName) {
    return function() {
	var allArgs = Array.prototype.slice.call(arguments);
	var args = allArgs.slice(0, allArgs.length-1);
	var cb = allArgs[allArgs.length-1];
	self.call(functionName, args, cb);
    }
}

// Conveniency for registering calls
RPCManager.prototype.setCalls = function(functionNames) {
    var self = this;
    for (var i = 0; i < functionNames.length; i++) {
	var functionName = functionNames[i];
	assert(typeof functionName == 'string');
	self[functionName] = makeRpcCall(self, functionName);
    }
}

// Create the manager that will be exported from this module.
var manager = new RPCManager();

// Initiate the BT.
noble.on('discover', function(peripheral) {
    console.log('found peripheral:', peripheral.advertisement);
    peripheral.connect(function(err) {
	peripheral.discoverServices([rpcServiceUuid], function(err, services) {
	    services.forEach(function(service) {
		console.log('found service:', service.uuid);
		service.discoverCharacteristics([], function(err, characteristics) {


		    var found = false;
		    characteristics.forEach(function(characteristic) {
			console.log('found characteristic:', characteristic.uuid);

			if (rpcCharacteristicUuid == characteristic.uuid) {
			    manager.deferredCharacteristic.resolve(characteristic);
			    found = true;
			}
		    });
		    
		    if (!found) {
			console.log('RPC characteristic not found');
		    }
		})
	    })
	})
    })
})

module.exports = manager;
