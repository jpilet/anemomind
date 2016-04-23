'use strict';

var express = require('express');
var http = require('http');
var path = require('path');
var app = exports.app = express();

app.set('port', process.env.PORT || 3000);
app.set('views', __dirname + '/dist');
app.set('view engine', 'ejs');
//app.use(express.favicon());
//app.use(express.logger('dev'));
//app.use(express.bodyParser());
//app.use(express.methodOverride());

var cookieParser = require('cookie-parser')
var errorHandler = require('errorhandler')

app.use(cookieParser());

//app.use(express.cookieParser('my-super-secret-123'));
//app.use(express.compress());
app.use(express.static(path.join(__dirname, '/dist')));
// Render *.html files using ejs
app.engine('html', require('ejs').__express);

app.use(errorHandler());

var env = process.env.NODE_ENV || 'development';
if ('development' == env) {
  var exec = require('child_process').exec;
  exec('node_modules/brunch/bin/brunch watch', function callback(error, stdout, stderr) {
    if (error) {
      console.log('An error occurred while attempting to start brunch.\n' +
                  'Make sure that it is not running in another window.\n');
      throw error;
    }
  });
};


// Routes //
app.get('/', function (req, res) {
  res.render('index.html');
});


// Catch all route -- If a request makes it this far, it will be passed to angular.
// This allows for html5mode to be set to true. E.g.
// 1. Request '/signup'
// 2. Not found on server.
// 3. Redirected to '/#/signup'
// 4. Caught by the '/' handler passed to Angular
// 5. Angular will check for '#/signup' against it's routes.
// 6. If found
//    a. Browser supports history api -- change the url to '/signup'.
//    b. Browser does not support history api -- keep the url '/#/signup'
app.use(function (req, res) {
  res.redirect('/#' + req.url);
});

http.createServer(app).listen(app.get('port'), function () {
  console.log("Express server listening on port " + app.get('port'));
});

