var upload = require('../lib/controllers/upload'),
	id = process.argv[2],
	polar = process.argv[3],
	race = process.argv[4];

upload.storeUpload(id, polar, race);
// process.argv.forEach(function (val, index, array) {
//   console.log(index + ': ' + val);
// });