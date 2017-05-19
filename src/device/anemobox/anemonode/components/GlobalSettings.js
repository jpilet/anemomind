var path = require('path');

mailRoot = '/media/sdcard/mail2/';
sentLogsSubPath = 'sentlogs';

module.exports = {
  mailRoot: mailRoot,
  sentLogsSubPath: sentLogsSubPath,
  sentLogsPath: path.join(mailRoot, sentLogsSubPath)
};
