/**
 * Broadcast updates to client when the model changes
 */

'use strict';

var Upload = require('./upload.model');

exports.register = function(socket) {
  Upload.schema.post('save', function (doc) {
    onSave(socket, doc);
  });
  Upload.schema.post('remove', function (doc) {
    onRemove(socket, doc);
  });
}

function onSave(socket, doc, cb) {
  socket.emit('upload:save', doc);
}

function onRemove(socket, doc, cb) {
  socket.emit('upload:remove', doc);
}