
module.exports.number = (s, decimals) => {
  if (isFinite(s)) {
    const fixed = parseFloat(s).toFixed(decimals);
    return fixed.match(/(.*?)\.?0*$/)[1];
  }
  return s;
};
