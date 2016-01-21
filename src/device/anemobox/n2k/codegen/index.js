var gen = require('./gen.js');

inputPath = '/home/jonas/programmering/cpp/canboat/analyzer/pgns.xml';
outputPath = '/home/jonas/programmering/sailsmart/src/device/anemobox/n2k';

gen.generate(inputPath, outputPath, function(err, value) {
  if (err) {
    console.log("Failed to generate because ");
    console.log(err);
  } else {
    console.log("Success!");
  }
});
