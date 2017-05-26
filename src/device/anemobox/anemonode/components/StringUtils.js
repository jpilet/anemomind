var assert = require('assert');

// Concatenates incoming strings,
// then splits them again at 'word'
function catSplit(word) {
  return function(red) {
    var partOfString = '';
    return function(dst, bufferChunk) {
      //console.log('dst = ' + dst);
      //console.log('bufferChunk = ' + bufferChunk);
      assert((bufferChunk instanceof Buffer) 
             || typeof bufferChunk == 'string');
      var parts = ('' + partOfString + bufferChunk).split(word);
      if (parts.length == 0) {
        return dst;
      } else {
        var m = parts.length - 1;
        partOfString = parts[m];
        return parts.slice(0, m).reduce(red, dst);
      }
    };
  }
}

module.exports.catSplit = catSplit;
