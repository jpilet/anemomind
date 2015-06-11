/**
 * Broadcast updates to client when the model changes
 */

'use strict';

var Script = require('./script.model');

exports.register = function(socket) {
  Script.schema.post('save', function (doc) {
    onSave(socket, doc);
  });
  Script.schema.post('remove', function (doc) {
    onRemove(socket, doc);
  });
}

function onSave(socket, doc, cb) {
  socket.emit('script:save', doc);
}

function onRemove(socket, doc, cb) {
  socket.emit('script:remove', doc);
}