import { assert } from './assert.js';

export function debugcb(err, response, body) {
    console.log('err = %j', err);
    console.log('response = %j', response);
    console.log('body = %j', body);
}

// An object that represents a connection to a server.
export function ServerConnection(address, token) {
    this.address = address;
    this.authUrl = address + '/auth/local';
    this.mailRpcUrl = address + '/api/mailrpc';
    this.token = token;
}

// Method to login
ServerConnection.prototype.login = async function(userdata, cb) {
    assert(userdata.email);
    assert(userdata.password);
    var self = this;
    
    var opts = {
	method: 'POST',
        headers: new Headers({'content-type': 'application/json'}),
	body: JSON.stringify(userdata)
    };

    try {
        let result = await fetch(this.authUrl, opts)
        if (result.ok) {
            self.token = await result.json().token;
            cb();
        } else {
            cb (new Error("login failed: " + result.status));
        }
    } catch (err) {
        cb(err);
    }
}

export function toJson(method, data) {
    if (typeof data == 'string') {
	if (data.length == 0) {
	    return undefined;
	}
	return JSON.parse(data);
    }
    return data;
}
    
function interpretResponse(method, err, response, rawBody, cb) {
    var body = toJson(method, rawBody);
    if (response.ok) {
	cb(undefined, body);
    } else {
	cb(body, undefined);
    }
}


ServerConnection.prototype.makePostRequest =
    async function(endpointName, method, dataToPost, cb) {
	let self = this;
	const url = (self.mailRpcUrl + '/' + method.name + '/' + endpointName);
	var opts = {
	    method: 'POST',
	    body: JSON.stringify(dataToPost),
	    headers: {
		Authorization: "Bearer " + self.token
	    }
	};
        try {
            result = await fetch(url, opts);

            try {
                cb(undefined, await result.json());
            } catch {
                cb(undefined, await result.blob());
            }
        } catch(err) {
            cb(err);
        }
	request(opts, function(err, response, body) {
	    interpretResponse(method, err, response, body, cb);
	});
    }



ServerConnection.prototype.makeGetRequest =
    function(endpointName, method, args, cb) {
	var self = this;
	var opts = {
	    url: (self.mailRpcUrl + '/' + method.name + '/' + endpointName + '/' + args),
	    method: 'GET',
	    headers: {
		Authorization: "Bearer " + self.token
	    }
	};
	request(opts, function(err, response, body) {
	    interpretResponse(method, err, response, body, cb);
	});
    }
