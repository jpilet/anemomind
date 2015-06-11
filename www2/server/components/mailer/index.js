/**
 * Mailer component
 */

'use strict';

var nodemailer = require('nodemailer');
var transporter = nodemailer.createTransport({
    service: 'gmail',
    auth: {
        user: 'anemobot@gmail.com',
        pass: 'an3m0b0t!'
    }
});

exports.transporter = transporter;