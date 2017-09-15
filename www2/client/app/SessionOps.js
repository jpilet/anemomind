(function(exports){

  function toDate(x) {
    return x instanceof Date? x : new Date(x);
  }

  function shallowCopy(x) {
    var y = {};
    for (var k in x) {
      y[k] = x[k];
    }
    return y;
  }

  function normalizeSession(session) {
    var dst = shallowCopy(session)
    dst.startTime = toDate(session.startTime);
    dst.endTime = toDate(session.endTime);
    return dst;
  }

  function sessionDurationSeconds(session) {
    return 0.001*(session.endTime.getTime() 
                  - session.startTime.getTime());
  }

  // Used to construct a binary session node.
  // If any argument is null
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

  // Helper function used by 'buildSessionTree':
  // Given an array of session nodes, 
  // Make pairs.
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

  // A session tree is a just a binary tree
  // of sessions, where each inner node summarizes
  // its subtree. That lets us to, on average, O(log(n))
  // edits on n sessions.
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
    if (!tree) {
      return dst;
    } else if (isLeaf(tree)) {
      return f(dst, tree);
    } else {
      var g = function(acc, x) {
        return reduceSessionTreeLeaves(f, acc, x);
      };
      return g(g(dst, tree.left), tree.right);
    }
  }

  function overlap(tree, edit) {
    if (tree.endTime <= edit.lower) {
      return false;
    } else if (edit.upper <= tree.startTime) {
      return false
    }
    return true;
  }

  function binaryDateOp(f, a, b) {
    return new Date(f(a.getTime(), b.getTime()));
  }

  function maxDate(a, b) {
    return binaryDateOp(Math.max, a, b);
  }
  
  function minDate(a, b) {
    return binaryDateOp(Math.min, a, b);
  }

  function cropSession(tree, lower, upper) {
    if (upper <= lower) {
      return null;
    }
    var dst = shallowCopy(tree);

    // It could be that deletion results in two new sessions
    // and then they should not have the same id.
    delete dst._id;

    dst.startTime = lower;
    dst.endTime = upper;
    return dst;
  }

  function applyDeleteToLeaf(tree, edit) {
    return binarySessionNode(
      cropSession(tree, tree.startTime, edit.lower),
      cropSession(tree, edit.upper, tree.endTime));
  }

  // Implementation of the delete operation.
  function applyDelete(tree, edit) {
    if (!overlap(tree, edit)) {
      return tree;
    } else if (isLeaf(tree)) {
      return applyDeleteToLeaf(tree, edit);
    } else {
      return binarySessionNode(
        applyDelete(tree.left, edit),
        applyDelete(tree.right, edit));
    }
  }

  // Applies a single edit operation to
  // a session tree.
  function applyEdit(tree, edit) {
    var ops = {
      "delete": applyDelete
    };
    var op = ops[edit.type];
    if (op) {
      return op(tree, edit);
    } else {
      console.log("Edit operation of type '"+edit.type+"' not recognized");
      return tree;
    }
  }

  exports.normalizeSession = normalizeSession; 
  exports.buildSessionTree = buildSessionTree;
  exports.reduceSessionTreeLeaves = reduceSessionTreeLeaves;
  exports.sessionDurationSeconds = sessionDurationSeconds;
  exports.applyEdit = applyEdit;

})(typeof exports === 'undefined'? this['SessionOps']={}: exports);
