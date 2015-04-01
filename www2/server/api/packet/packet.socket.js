/**
 * Broadcast updates to client when the model changes
 */

'use strict';

var Packet = require('./packet.model');

exports.register = function(socket) {
  Packet.schema.post('save', function (doc) {
    onSave(socket, doc);
  });
  Packet.schema.post('remove', function (doc) {
    onRemove(socket, doc);
  });
}

function onSave(socket, doc, cb) {
  socket.emit('packet:save', doc);
}

function onRemove(socket, doc, cb) {
  socket.emit('packet:remove', doc);
}