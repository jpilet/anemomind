function MutableString() {
    this.data = '';
}

MutableString.prototype.add = function(s) {
    this.data = this.data + s;
};

MutableString.prototype.get = function() {
    return this.data;
};


function TreeInfo(stateCount, labels) {
    this.stateCount = stateCount;
    this.labels = labels;
}

TreeInfo.prototype.renderTreeAsTextSub = function(at, depth, tree, maxDepth, navdata, accstring) {
    assert(isSailData(navdata), 'renderTreeAsTextSub: not saildata');
    var type = tree[0];
    var args = tree.slice(1, tree.length);
    var indent = repeatChar('       ', depth);
    var output = depth <= maxDepth;
    if (type < this.stateCount) { // Terminal
	var count = args[0];
	var next = at + count;
	if (output) {
	    
	    var timedata = '';
	    if (false) {
		var dur = getSecond(navdata[next-1]) -
	  getSecond(navdata[at]);
		
		timedata =  '  for ' +
		    + dur + 
		    ' seconds (from ' + getTimeString(navdata[at]) +
		    '  to ' + getTimeString(navdata[next-1])
		    + ')';
	    }
	    //document.writeln();
	    accstring.add(indent + this.labels[type] + timedata + '\n');


// + '  [' + at + ', ' + next + '[');
			     
	}
	return count;
    }
    else {
	var deeper = depth + 1;
	var count = 0;
	if (output) {
	    //document.writeln(indent + this.labels[type]);
	    accstring.add(indent + this.labels[type] + '\n');


	}
	var argc = args.length;
	for (var i = 0; i < argc; i++) {
	    count += this.renderTreeAsTextSub(at + count, deeper,
					 args[i], maxDepth, navdata, 
					     accstring);
	}
	if (depth == 2) {
	    var next = at + count;
	    var timedata = '';
	    var dur = getSecond(navdata[next-1]) -
	     getSecond(navdata[at]);
	    
	    timedata = '  (from ' + getTimeString(navdata[at]) +
		'  to ' + getTimeString(navdata[next-1])
		+ ')';
	    //document.writeln(indent + timedata);
	    accstring.add(indent + timedata + '\n');
	}

	return count;
    }
};


TreeInfo.prototype.renderTreeAsText = function(tree, maxDepth,
					       navdata, accstring) {
    assert(isSailData(navdata), 'renderTreeAsText: not saildata');
    //document.write('<pre>');
    accstring.add('<pre>');
    this.renderTreeAsTextSub(0, 0, tree, maxDepth, navdata, accstring);
    //document.write('</pre>');
    accstring.add('</pre>');
    
};

TreeInfo.prototype.render = function(tree, maxDepth, navdata) {
    assert(isSailData(navdata), 'render: not saildata');
    if (!isDefined(maxDepth)) {
	maxDepth = 119;
    }
    var accstring = new MutableString();
    this.renderTreeAsText(tree, maxDepth, navdata, accstring); 
    return accstring.get();
};

// function withSubtreeSizeSub(tree, offset) {
//     var type = tree[0];
//     var args = tree.slice(1, tree.length);
//     var argc = args.length;
    
//     var isLeaf = false;
//     if (argc == 1) {
// 	if (is
//     }

//     if (isLeaf) {
// 	return [type [s offset 1]]
//     }
//     else {
//     }

//     var newArgs = new Array(argc + 1);

    

//     for (var i = 0; i < argc; i++) {
// 	var subnode
//     }
// }




// // Extends the tree to contain the intervals
// function withSubtreeSize(tree) {
//     return withSubtreeSizeSub(0, tree);    
// }






function TreeStyle(beginTree, endTree, beginInnerNode, endInnerNode, leaf) {
    this.beginTree = beginTree;
    this.endTree = endTree;
    this.beginInnerNode = beginInnerNode;
    this.endInnerNode = endInnerNode;
    this.leaf = leaf;
}

function isTreeStyle(x) {
    return isDefined(x.beginTree) &&
	isDefined(x.endTree) &&
	isDefined(x.beginInnerNode) &&
	isDefined(x.endInnerNode) &&
	isDefined(x.leaf);
};

function makeBasicTreeStyle() {
    var beginTree = function(s) {
	s.add('<li class="flist showlines">');
    };

    var endTree = function(s) {
	s.add('</li>');
    };

    var beginInnerNode = function(s) {
	s.add('<li><a href="#">Inner node</a><ul>');
    };

    var endInnerNode = function(s) {
	s.add('</ul></li>');
    };

    var leaf = function(s) {
	s.add('<li><a href="#">Leaf</a></li>');
    };

    return new TreeStyle(beginTree, endTree, beginInnerNode, endInnerNode, leaf);
}


TreeInfo.prototype.renderExpandable = function(tree, navdata) {
    
};
