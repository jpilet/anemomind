'use strict';
var mongoose = require('mongoose');
var env = require('../config/environment');

module.exports = function(app) {
	// exported methods
	var controller={};

	let oghtml= function(ogdata) {
          var r = ' <!-- OPEN GRAPH -->\n'
                + ' <meta property="op:markup_version" content="v1.0">\n'
                + ' <link rel="canonical" href="' + ogdata.url + '" >\n'
                + ' <meta property="fb:app_id" content="'
                + env.facebook.clientID + '" />\n';
          for (var i in ogdata) {
            r += ` <meta property="og:${i}" content="${ogdata[i]}" >\n`;
          }
          return r;
        };

	// preload index.html
	var index=require('fs').readFileSync(app.get('appPath') + '/index.html').toString();


	// TODO replace this code with require('VectorTileLayer').curveEndTimeStr
        function curveEndTimeStr(curveId) {
          return curveId.substr(curveId.length-19);
        }

        // TODO replace this code with require('VectorTileLayer')
        function curveStartTimeStr(curveId) {
          return curveId.substr(curveId.length-19*2,19);
        }

	// open a shared map
	controller.get=function (req,res) {

	  var l = req.query.l;
	  var boatId = req.params.boatId;
	  var curve = req.query.c;
	  var start = curve ? curveStartTimeStr(curve) : undefined;
	  var end = curve ? curveEndTimeStr(curve) : undefined;
	  var width = 1024, height=536;

	  // build image url that keep ideal FB ratio 1.91:1 (600x315, 800x418, 1024x536)
	  var ogImg = [
            "/api/map",
            boatId,
            l || '',
            start ||'',
            end || '',
            width+"-"+height+".png"
          ].join('/');

	  // Load current boat
	  mongoose.model('Boat').findById(boatId).select('name').exec(function(err,boat) {
	  	if(err){
	  		return res.status(500).send(err);
	  	}

	  	// build shared url
	  	var hostname = 'https://www.anemolab.com';
	  	var port = req.header('x-forwarded-port');
	  	if (port && port != 443) {
	  	  hostname += ':' + port; // running beta
	  	}

	  	// prepare social data model
	  	// TODO make this information more accurate
		  var model={
                    // TODO: decompose and recompose the URL so that parameter
                    // ordering never change
                    canonical: hostname + req.originalUrl,
                    url:hostname + req.originalUrl,
                    title: boat.name +' sailing ' + new Date(start).toDateString(),
                    description:
                      boat.name +' sailing on ' + new Date(start).toDateString(),
                    "image": hostname+ogImg,
                    "image:secure_url": hostname+ogImg,
                    "image:type": "image/png",
                    "image:width": width,
                    "image:height": height
		  }

                  // send content
		  res.send(index.replace('<!-- OPEN GRAPH -->', oghtml(model)));
	  });
	}

	return controller;
}
