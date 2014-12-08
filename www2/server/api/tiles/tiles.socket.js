/**
 * Broadcast updates to client when the model changes
 */

'use strict';

var Tiles = require('./tiles.model');

exports.register = function(socket) {
  Tiles.schema.post('save', function (doc) {
    onSave(socket, doc);
  });
  Tiles.schema.post('remove', function (doc) {
    onRemove(socket, doc);
  });
}

function onSave(socket, doc, cb) {
  socket.emit('tiles:save', doc);
}

function onRemove(socket, doc, cb) {
  socket.emit('tiles:remove', doc);
}