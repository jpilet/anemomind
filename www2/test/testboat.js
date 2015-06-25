var Boat = require('../server/api/boat/boat.model.js');
var removeBoat = true;

function withTestBoat(cbOperation, cbDone) {
  Boat.create({
    name: 'Frida',
    type: 'IF',
    sailNumber: '1604',
    anemobox: 'abc119'}, function(err, docInserted) {
      var id = docInserted._id;
      cbOperation(id, function(err) {
        if (removeBoat) {
          Boat.remove({_id: id}, function(err2) {
            cbDone(err || err2);
          });
        } else {
          cbDone(err);
        }
      });
    });
}

module.exports = withTestBoat;
