/**
 * Broadcast updates to client when the model changes
 */

'use strict';

var BoxExec = require('./boxexec.model');

exports.register = function(socket) {
  BoxExec.schema.post('save', function (doc) {
    onSave(socket, doc);
  });
  BoxExec.schema.post('remove', function (doc) {
    onRemove(socket, doc);
  });
}

function onSave(socket, doc, cb) {
  socket.emit('boxexec:save', doc);
}

function onRemove(socket, doc, cb) {
  socket.emit('boxexec:remove', doc);
}
