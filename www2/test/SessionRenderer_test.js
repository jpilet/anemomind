var fs = require('fs');
var assert = require('assert');

var _ = require('lodash');
var SessionOps = require('../client/app/SessionOps.js');
var anemoutils = require('../client/app/anemoutils.js');

// Not sure how to require JS code that has not been wrapped in a module.
eval(fs.readFileSync('../client/app/boatList/SessionRenderer.js', 'utf8'));

var sessions = require('./raw_sessions.json');

describe('SessionRenderer', function() {
  it('Test it was loaded', function() {
    assert(SessionRenderer);
  });

  it('Test basic ops', function() {
    var r = new SessionRenderer();
    for (var i in sessions) {
      r.addSession(sessions[i]);
    }

    assert(r.renderedArray.get().length == sessions.length);
  });
});

