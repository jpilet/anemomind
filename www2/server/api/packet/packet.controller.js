'use strict';

var _ = require('lodash');
var Packet = require('./packet.model');

// Get list of packets
exports.index = function(req, res) {
  Packet.find(function (err, packets) {
    if(err) { return handleError(res, err); }
    return res.json(200, packets);
  });
};

// Get a single packet
exports.show = function(req, res) {
  Packet.findById(req.params.id, function (err, packet) {
    if(err) { return handleError(res, err); }
    if(!packet) { return res.send(404); }
    return res.json(packet);
  });
};

// Creates a new packet in the DB.
exports.create = function(req, res) {
  Packet.create(req.body, function(err, packet) {
    if(err) { return handleError(res, err); }
    return res.json(201, packet);
  });
};

// Updates an existing packet in the DB.
exports.update = function(req, res) {
  if(req.body._id) { delete req.body._id; }
  Packet.findById(req.params.id, function (err, packet) {
    if (err) { return handleError(res, err); }
    if(!packet) { return res.send(404); }
    var updated = _.merge(packet, req.body);
    updated.save(function (err) {
      if (err) { return handleError(res, err); }
      return res.json(200, packet);
    });
  });
};

// Deletes a packet from the DB.
exports.destroy = function(req, res) {
  Packet.findById(req.params.id, function (err, packet) {
    if(err) { return handleError(res, err); }
    if(!packet) { return res.send(404); }
    packet.remove(function(err) {
      if(err) { return handleError(res, err); }
      return res.send(204);
    });
  });
};

function handleError(res, err) {
  return res.send(500, err);
}