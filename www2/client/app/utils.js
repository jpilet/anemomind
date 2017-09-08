function accessKey(m, key, init) {
  console.log("Great stuff!");
  if (!(key in m)) {
    m[key] = init;
  }
  return m[key];
}

function fatalError(x) {
  alert('FATAL ERROR: %j', x); 
}

function assert(x, msg) {
  if (!x) {
    fatalError("Assertion failed: '%s'", msg);
  }
}
