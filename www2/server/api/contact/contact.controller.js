'use strict';

var _ = require('lodash');
var User = require('../user/user.model');
var mongoose = require('mongoose');


var winston = require('winston');
var mailer = require('../../components/mailer');
var transporter = mailer.transporter;

var sendContactMessage = function(name, addr, subject, message, cb) {
  var from = name + "<" + addr + ">";
  var messageBody = "The contact form on regattapolar has been filled with:\n"
  + "From: " + from + "\n"
  + "Subject: " + subject + "\n"
  + "Message:\n" + message + "\n";

  transporter.sendMail({
    from: mailer.from,
    to: "info@astrayacht.com, julien@anemomind.com",
    subject: 'Contact form: ' + subject,
    text: message
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
                     function(err) {
    if (err) {
      res.sendStatus(400);
    } else {
      res.sendStatus(201);
    }
  });
};
