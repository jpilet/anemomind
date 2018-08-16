expandFBinArray = function(compact) {
  const result = [];

  if (compact.buffer && compact.buffer.length) {
    for (let i = 0; i < compact.buffer.length; i += 4) {
      result.push(compact.buffer.readFloatLE(i));
    }
  }
  return result;
}

module.exports.expandArrays = function(obj) {
  const keys = Object.keys(obj);
  const result = { };

  console.log('expandArrays');

  for (let k of keys) {
    const match = k.match(/(.*)_fbin$/);
    if (match) {
      result[match[1]] = expandFBinArray(obj[k]);
    } else {
      result[k] = obj[k];
    }
  }
  return result;
}

