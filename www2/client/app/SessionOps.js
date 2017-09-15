(function(exports){

  function pairUp(src0) {
    var src = src0;
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

  function buildTree(arr) {
    var dst = arr;
    while (1 < dst.length) {
      dst = pairUp(dst);
    }
    return dst;
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

})(typeof exports === 'undefined'? this['anemoutils']={}: exports);
