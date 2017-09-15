(function(exports){

  function unwrapMaximally(x) {
    return x.length == 1? unwrapMaximally(x[0]) : x;
  }

  // Given an array of items, build pairs.
  // Removes unnecessary wrapping and, if 
  // the array is already a pair, returns it.
  function pairUp(src0) {
    var src = unwrapMaximally(src0);
    if (src.length <= 2) {
      return src;
    }

    var dst = [];
    while (2 <= src.length) {
      dst.push(src.slice(0, 2));
      src = src.slice(2);
    }
    if (1 == src.length) {
      dst.push(src[0]);
    }
    return dst;
  }
  
  // Constructs a binary tree from an array
  function buildTree(arr) {
    var dst = arr;
    while (2 < dst.length) {
      dst = pairUp(dst);
    }
    return dst;
  }

  // Flattens a tree into 'dst', by repeatedly applying 
  // f(dst, x) with x being the next element in the tree,
  // depth first, left-to-right.
  function flattenTree(f, dst, src) {
    return (!(src instanceof Array))?
      f(dst, src) : src.reduce(function(acc, x) {
        return flattenTree(f, acc, x)
      }, dst);
  }

  function Editor() {

    // Maps session id to session data
    this.sessionMap = {};

    // Edit operations in chronological order
    this.edits = [];

    // A tree structure used to apply the edits
    this.sessionTree = null;
  }

  Editor.prototype.addSession = function(session) {
    if (session._id in this.sessionMap) {
      return;
    }
    this.sessionTree = null;
    this.sessionMap[session._id] = session;
  }

  Editor.prototype.addEdit = function(edit) {
    this.edits.push(edit);
    this.sessionTree = applyEdit(this.sessionTree, edit);
  }


  exports.buildTree = buildTree;
  exports.flattenTree = flattenTree;

})(typeof exports === 'undefined'? this['anemoutils']={}: exports);
