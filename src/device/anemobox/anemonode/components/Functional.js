function filter(f) {
  return function(red) {
    return function(dst, x) {
      return f(x)? red(dst, x) : dst;
    }
  };
}

function map(f) {
  return function(red) {
    return function(dst, x) {
      return red(dst, f(x));
    }
  };
}

function compose2(fa, fb) {
  return function(x) {return fa(fb(x));}
}

function compose() {
  var args = Array.prototype.slice.call(arguments);
  return args.reduce(compose2);
}

module.exports.compose = compose;
module.exports.filter = filter;
module.exports.map = map;
