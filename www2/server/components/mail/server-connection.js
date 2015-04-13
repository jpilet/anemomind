var assert = require('assert');
var request = require('request');
var JSONB = require('json-buffer');

function debugcb(err, response, body) {
    console.log('err = %j', err);
    console.log('response = %j', response);
    console.log('body = %j', body);
}

// An object that represents a connection to a server.
function ServerConnection(address, token) {
    this.address = address;
    this.authUrl = address + '/auth/local';
    this.mailRpcUrl = address + '/api/mailrpc';
    this.token = token;
}

// Method to login
ServerConnection.prototype.login = function(userdata, cb) {
    assert(userdata.email);
    assert(userdata.password);
    var self = this;
    
    var opts = {
	url: this.authUrl,
	method: 'POST',
	json: userdata
    };

    request(
	opts,
	function(err, response, body) {
	    if (err) {
		cb(err);
	    } else {
		if (response.statusCode == 200) {
		    self.token = body.token;
		    cb(
			undefined
		    );
		} else {
		    cb(
			new Error('Unexpected status code: ' + response.statusCode)
		    );
		}
	    }
	}
    );
}

ServerConnection.prototype.makePostRequest =
    function(mailboxName, methodName, dataToPost, cb) {
    var self = this;
    var opts = {
	url: (self.mailRpcUrl + '/' + methodName + '/' + mailboxName),
	method: 'POST',
	json: dataToPost,
	headers: {
	    Authorization: "Bearer " + self.token
	}
    };
    request(opts, function(err, response, body) {
	cb(err, body)
    });
}


module.exports = ServerConnection;
