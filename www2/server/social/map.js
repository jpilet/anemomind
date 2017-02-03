'use strict';
var mongoose = require('mongoose');
module.exports = function(app) {
	// exported methods
	var controller={};

	let oghtml= function(ogdata) {
          var r = '<!-- OPEN GRAPH -->\n';
          for (var i in ogdata) {
            r += `<meta property="og:${i}" content="${ogdata[i]}"/>\n`;
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

	  var l=req.query.l, 
	      boatId=req.params.boatId,
	      curve=req.query.c, 
	      start=curveStartTimeStr(curve),
	      end=curveEndTimeStr(curve),
	      width=1024, height=536;

	  // build image url that keep ideal FB ratio 1.91:1 (600x315, 800x418, 1024x536)
	  var ogImg=[
	    "/api/map",boatId,l,start,end,width+"-"+height+".png"
	  ].join('/');

	  // Load current boat
	  mongoose.model('Boat').findById(boatId).select('name').exec(function(err,boat) {
	  	if(err){
	  		return res.status(500).send(err);
	  	}

	  	// build shared url 
	  	var hostname = req.protocol + '://' + req.headers.host;

	  	// prepare social data model
	  	// TODO make this information more accurate
		  var model={
		    url:hostname+req.url,
		    title: boat.name +' sailing '+new Date(start).toDateString(),
		    description: boat.name +' sailing on '
                        + new Date(start).toDateString(),
		    image:hostname+ogImg,
		    width:width,
		    height:height
		  }

                  // send content
		  res.send(index.replace('<!-- OPEN GRAPH -->', oghtml(model)));	
	  });
	}

	return controller;
}
