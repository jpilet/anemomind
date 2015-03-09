/**
 * Broadcast updates to client when the model changes
 */

'use strict';

var Boat = require('./boat.model');

exports.register = function(socket) {
  Boat.schema.post('save', function (doc) {
    onSave(socket, doc);
  });
  Boat.schema.post('remove', function (doc) {
    onRemove(socket, doc);
  });
}

function onSave(socket, doc, cb) {
  socket.emit('boat:save', doc);
}

function onRemove(socket, doc, cb) {
  socket.emit('boat:remove', doc);
}