/**
 * Mailer component
 */

'use strict';

var nodemailer = require('nodemailer');
var config = require('../../config/environment');

var transporter = nodemailer.createTransport(config.smtp);

exports.transporter = transporter;
exports.from = '"Anemomind" <anemolab@anemomind.com>';
