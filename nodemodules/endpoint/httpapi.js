var express = require('express');

/* 
Make a router for accessing an endpoint.
In informal syntax, 

accessEndpoint: ((endpoint, (err)->()), (err)->())

accessEndpoint is a function that accepts two arguments:

  * The first argument is a function
    that takes as input the endpoint that we want to access, as well
    as a callback function that it will call once it is done.

  * The second argument is a callback function that is called once the
    the endpoint is no longer accessed, and program flow should move on.

*/
function make(accessEndpoint) {
  var router = express.Router();
  router.get('/info', function(req, res) {
    res.send('endpoint');
  });
  return router;
}

module.exports.make = make;
