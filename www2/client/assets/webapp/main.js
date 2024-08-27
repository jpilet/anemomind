import { schema } from "./endpoint-schema.js";
import { ServerConnection } from "./server-connection.js";
import { tryMakeEndpoint  } from "./endpoint.httpclient.js";

export function main(el) {
    el.innerHTML="Trying to start endpoint...";

    tryMakeEndpoint(window.location.origin, {email: 'test@a.com', password: 'toto123' },
                    '66c96aae6f3ae3001a2ab49e', function(err, endpoint) {
                        if (err) {
                            el.innerHTML = '' + err + '<br/>' + err.stack;
                        } else {
                            el.innerHTML = 'success!';
                        }
                    });
}
