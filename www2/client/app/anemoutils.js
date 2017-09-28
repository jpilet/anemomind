(function(exports){

  function push(dst, x) {
    dst.push(x);
    return dst;
  }

  function add(a, b) { // Because '+' cannot be used as a function.
    return a + b;
  }

  // Lets us generalize the operation of 'map' to many situations.
  function map(f) { // Returns a  mapping transducer
    return function(r /*old reducing function*/) { // The transducer
      return function(acc /*accumulated value*/, x/*new value*/) { // The new reducing function
        return r(acc, f(x));
      }
    }
  }

  function fatalError(x) {
    var msg = "FATAL ERROR: " + (x || "(no message defined)");
    console.log(msg);
    console.log(new Error(msg).stack);
  }

  function assert(x, msg) {
    if (!x) {
      fatalError("Assertion failed: " + msg);
    }
  }

  function allocateFieldIfNeeded(obj, key) {
    if (!(key in obj)) {
      obj[key] = {};
    }
  }

  // Set a value deeply in a datastructure
  function setIn(dst, path, value) {
    if (path.length == 0) {
      return value;
    }
    var root = dst || {};
    var obj = root;
    var last = path.length - 1;
    for (var i = 0; i < last; i++) {
      var key = path[i];
      allocateFieldIfNeeded(obj, key);
      obj = obj[key];
    }
    var key = path[last];
    obj[key] = value;
    return root;
  };

  // Get a value from deep inside a datastructure
  function getIn(src, path) {
    var obj = src;
    for (var i in path) {
      if (!obj) {
        return null;
      }
      var key = path[i];
      obj = obj[key];
    }
    return obj;
  };

  // Update a value in a deep data structure
  function updateIn(dst, path, f) {
    // Not optimized, but simple.
    // TODO: Optimize it.
    return setIn(dst, path, f(getIn(dst, path)));
  }

  // ValueState is used to facilitate caching, so
  // that we only recompute things when they are
  // needed.
  //
  // As arguments, it accepts a function, that 
  // will compute its value, and an array of 
  // other ValueState objects from which it gets
  // those values.
  function ValueState(f, args) {
    assert(!f || typeof f == "function", "ValueState, f should be a function");
    this.value = null;
    this.version = 0;
    this.f = f;
    this.args = (args || []).map(function(valueState) {
      if (!(valueState instanceof ValueState)) {
        fatalError("Not a ValueState!");
      }
      return {
        'version': null,
        'valueState': valueState
      };
    });
  }

  ValueState.prototype.set = function(newValue) {
    this.value = newValue;
    this.version += 1;
  }

  ValueState.prototype.isUpToDate = function() {
    for (var i in this.args) {
      var arg = this.args[i];
      if (arg.version != arg.valueState.version 
          || !arg.valueState.isUpToDate()) {
        return false;
      }
    }
    return true;
  }

  ValueState.prototype.get = function() {
    if (this.isUpToDate()) {
      return this.value;
    }
    var args = new Array(this.args.length);
    for (var i in this.args) {
      var arg = this.args[i];
      args[i] = arg.valueState.get();
      arg.version = arg.valueState.version;
    }
    if (this.f) {
      this.value = this.f.apply(null, args);
    }
    return this.value;
  }

  ValueState.prototype.update = function(f) {
    this.set(f(this.get()));
  }

  exports.map = map;
  exports.getIn = getIn;
  exports.setIn = setIn;
  exports.updateIn = updateIn;
  exports.assert = assert;
  exports.fatalError = fatalError;
  exports.push = push;
  exports.add = add;
  exports.ValueState = ValueState;

})(typeof exports === 'undefined'? this['anemoutils']={}: exports);
