// Equivalent prices, in different monetary units
// E.g: 1 CHF = 0.86 EUR
var muTable = {
  chf: 1.0,
  eur: 0.86
}

var mu = invertMuTable(muTable);

var prices = [
  ["buy-box", 850*mu.chf],
  ["rent-box-annual", 200*mu.chf],
  ["rent-box-deposit", 300*mu.chf],
  ["subs-social-annual", 0*mu.chf],
  ["subs-social-life", 0*mu.chf],
  ["subs-amateur-annual", 100*mu.chf],
  ["subs-amateur-life", 500*mu.chf],
  ["subs-pro-annual", 300*mu.chf],
  ["subs-pro-life", 1500*mu.chf],
  ["wind-sensor", 665*mu.chf],
  ["ipad", 479*mu.chf],
  ["waterproof", 80*mu.chf]
];



////////////// Implementation
function invertMuTable(x) {
  var y = {};
  for (var k in x) {
    y[k] = 1.0/x[k];
  }
  return y;
}


function roundOff(x) {
  var mul = 10;
  return mul*Math.round(x/mul)
}

function renderPrices(unitKey) {
  var unit = mu[unitKey];
  for (var k in prices) {
    var data = prices[k];
    console.log("%s: %j %s", data[0], roundOff(data[1]/unit), unitKey);
  }
}

renderPrices("eur");
