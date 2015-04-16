/**
 * Broadcast updates to client when the model changes
 */

'use strict';

var boat = require('./boat.model');

exports.register = function(socket) {
  boat.schema.post('save', function (doc) {
    onSave(socket, doc);
  });
  boat.schema.post('remove', function (doc) {
    onRemove(socket, doc);
  });
}

function onSave(socket, doc, cb) {
  socket.emit('boat:save', doc);
}

function onRemove(socket, doc, cb) {
  socket.emit('boat:remove', doc);
}
