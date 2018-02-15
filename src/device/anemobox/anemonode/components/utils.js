function tag(value, unit) {
  if (value == null) {
    return null;
  }
  return [value, unit];
}

module.exports.tag = tag;
