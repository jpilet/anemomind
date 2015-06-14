var config = require('../components/config.js');



function emptyString(x) {
  if (typeof x == 'string') {
    return x.length == 0;
  }
  return false;
}

function configHasData(cfg) {
  return !emptyString(cfg.boatId) && !emptyString(cfg.boatName);
}

// Just to make sure that there is meaningful
// information on the local computer where the
// tests are performed.
function ensure(cb) {
  config.get(function(err, cfg) {
    if (err) {
      cb(err);
    } else {
      if (configHasData(cfg)) {
        cb(null, cfg);
      } else {
        var testConfig = {boatName: "Irene", boatId:"119"};
        config.write(testConfig, function(err) {
          if (err) {
            cb(err);
          } else {
            cb(null, testConfig);
          }
        });
      }
    }
  });
}

module.exports = ensure;
