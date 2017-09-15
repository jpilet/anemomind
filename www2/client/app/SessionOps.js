(function(exports){

  function toDate(x) {
    return x instanceof Date? x : new Date(x);
  }

  function normalizeSession(session) {
    session.startTime = toDate(session.startTime);
    session.endTime = toDate(session.endTime);
    return session;
  }

  function sessionDurationSeconds(session) {
    return 0.001*(session.endTime.getTime() 
                  - session.start.getTime());
  }

  function binarySessionNode(left, right) {
    if (!left) {
      return right;
    }
    if (!right) {
      return left;
    }
    return {
      startTime: left.startTime,
      endTime: right.endTime,
      left: left,
      right: right
    };
  }

  function isLeaf(x) {
    return !x.left;
  }

  function pairUp(nodes) {
    if (nodes.length <= 1) {
      return nodes;
    }
    var dst = [];
    while (2 <= nodes.length) {
      dst.push(binarySessionNode(nodes[0], nodes[1]));
      nodes = nodes.slice(2);
    }
    if (nodes.length == 1) {
      dst.push(nodes[0]);
    }
    return dst;
  }

  function buildSessionTree(nodes0) {
    var nodes = nodes0;
    while (1 < nodes.length) {
      nodes = pairUp(nodes);
    }
    return nodes.length == 1? nodes[0] : null;
  }

  // Makes a depth-first left-to-right traversal of the
  // tree, accumulating the value dst by calling 
  // f(dst, x) on every visited leaf x.
  function reduceSessionTreeLeaves(f, dst, tree) {
    if (isLeaf(tree)) {
      return f(dst, tree);
    } else {
      var g = function(acc, x) {
        return reduceSessionTreeLeaves(f, acc, x);
      };
      return g(g(dst, tree.left), tree.right);
    }
  }

  exports.normalizeSession = normalizeSession; 
  exports.buildSessionTree = buildSessionTree;
  exports.reduceSessionTreeLeaves = reduceSessionTreeLeaves;
  exports.sessionDurationSeconds = sessionDurationSeconds;

})(typeof exports === 'undefined'? this['anemoutils']={}: exports);
