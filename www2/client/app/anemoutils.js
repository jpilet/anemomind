(function(exports){

  function allocateFieldIfNeeded(obj, key) {
    if (!(key in obj)) {
      obj[key] = {};
    }
  }

  exports.setIn = function(dst, path, value) {
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
  }

  

})(typeof exports === 'undefined'? this['mymodule']={}: exports);
