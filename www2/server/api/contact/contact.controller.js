'use strict';

var _ = require('lodash');
var User = require('../user/user.model');
var mongoose = require('mongoose');
const vhostForReq = require('../../components/vhost').vhostForReq;

var winston = require('winston');
var mailer = require('../../components/mailer');
var transporter = mailer.transporter;

var sendContactMessage = function(name, addr, subject, message, vhost, cb) {
  var from = name + "<" + addr + ">";

  const platform = {
    esalab: 'regattapolar',
    client: 'anemolab',
  };

  var messageBody = "The contact form on " + platform[vhost] + " has been filled with:\n"
  + "From: " + from + "\n"
  + "Subject: " + subject + "\n"
  + "Message:\n" + message + "\n";

  const dest = {
    client: "julien@anemomind.com",
    esalab: "info@astrayacht.com, julien@anemomind.com",
  };

  transporter.sendMail({
    from: mailer.from,
    to: dest[vhost] || dest.client,
    subject: 'Contact form: ' + subject,
    text: messageBody
  }, function(err, info) {
    if (err) {
      // TODO: update the info page and notify the inviter
      winston.log('error', 'Failed to send contact form message: '+ 
                         messageBody);
      console.warn('error', err);
      cb(err);
    } else {
      cb();
    }
  });
}

// Get list of boats
exports.sendContactMessage = function(req, res) {
  sendContactMessage(req.body.name + '', req.body.addr + '',
                     req.body.subject + '',
                     req.body.message + '',
                     vhostForReq(req),
                     function(err) {
    if (err) {
      res.sendStatus(400);
    } else {
      res.sendStatus(201);
    }
  });
};
