

function TreeInfo(stateCount, labels) {
    this.stateCount = stateCount;
    this.labels = labels;
}

TreeInfo.prototype.renderTreeAsTextSub = function(at, depth, tree, maxDepth, navdata, accstring) {
    assert(isSailData(navdata), 'renderTreeAsTextSub: not saildata');
    assert(tree.length >= 2, "A tree is an array where the first element is the type and the second element the number of states that it holds.");
    var type = tree[0];
    var size = tree[1];
    var args = tree.slice(2, tree.length);
    var indent = repeatChar('       ', depth);
    var output = depth <= maxDepth;
    if (args.length == 0) { // Terminal
	var next = at + size;
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
    }
    return size;
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






function TreeStyle(beginTree, endTree, beginInnerNode, endInnerNode, makeLeaf) {
    this.beginTree = beginTree;
    this.endTree = endTree;
    this.beginInnerNode = beginInnerNode;
    this.endInnerNode = endInnerNode;
    this.makeLeaf = makeLeaf;
}

function isTreeStyle(x) {
    return isDefined(x.beginTree) &&
	isDefined(x.endTree) &&
	isDefined(x.beginInnerNode) &&
	isDefined(x.endInnerNode) &&
	isDefined(x.makeLeaf);
};

function makeBasicTreeStyle() {
    var beginTree = function(tree, navdata, s) {
	s.add('<li class="flist showlines">');
    };

    var endTree = function(tree, navdata, s) {
	s.add('</li>');
    };

    var beginInnerNode = function(offset, tree, navdata, s) {
	s.add('<li><a href="#">Inner node</a><ul>');
    };

    var endInnerNode = function(offset, tree, navdata, s) {
	s.add('</ul></li>');
    };

    var makeLeaf = function(offset, tree, navdata, s) {
	s.add('<li><a href="#">Leaf</a></li>');
    };

    return new TreeStyle(beginTree, endTree, beginInnerNode, endInnerNode, makeLeaf);
}



TreeStyle.prototype.renderExpandableSub = function(offset, tree, navdata, s) {
    assert(isArray(tree));
    assert(isMutableString(s));
    assert(isSailData(navdata));
    var size = tree[1];
    var args = tree.slice(2, tree.length);
    if (args.length == 0) {
	this.makeLeaf(offset, tree, navdata, s);
    }
    else {
	this.beginInnerNode(offset, tree, navdata, s);
	var ioffs = offset;
	for (var i = 0; i < args.length; i++) {
	    ioffs += this.renderExpandableSub(ioffs, args[i], navdata, s);
	}
	this.endInnerNode(offset, tree, navdata, s);
    }
    return size;
};


TreeStyle.prototype.renderExpandable = function(tree, navdata) {
    var s = new MutableString();

    this.beginTree(tree, navdata, s);
    this.renderExpandableSub(0, tree, navdata, s);
    this.endTree(tree, navdata, s);

    return s.get();
};
