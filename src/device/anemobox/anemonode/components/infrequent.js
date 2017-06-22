function makeAcceptor(period) {
  var last = null;
  return function() {
    var now = new Date();
    if (!last || now - last >= period) {
      last = now;
      return true;
    } else {
      return false;
    }
  };
}

module.exports.makeAcceptor = makeAcceptor;
