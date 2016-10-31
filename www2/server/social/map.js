'use strict';
var express = require('express');
var config = require('../config/environment');
var User = require('../api/user/user.model');
var auth = require('../auth/auth.service');
var access = require('../api/boat/access');
var mongoose = require('mongoose');

module.exports = function(app) {

	//
	// preload index.html
	var index=require('fs').readFileSync(app.get('appPath') + '/index.html').toString();  


	var router = express.Router();

	// TODO replace this code with require('VectorTileLayer').curveEndTimeStr
  function curveEndTimeStr(curveId) {
    return curveId.substr(curveId.length-19);
  }

	// TODO replace this code with require('VectorTileLayer')
  function curveStartTimeStr(curveId) {
    return curveId.substr(curveId.length-19*2,19);
  }

	//
	// this is a simple template function 
	var template = function(html, options) {
	  var re = /<%([^%>]+)?%>/g, reExp = /(^( )?(if|for|else|switch|case|break|{|}))(.*)?/g, code = 'var r=[];\n', cursor = 0, match;
	  var add = function(line, js) {
	      js? (code += line.match(reExp) ? line + '\n' : 'r.push(' + line + ');\n') :
	          (code += line != '' ? 'r.push("' + line.replace(/"/g, '\\"') + '");\n' : '');
	      return add;
	  }
	  while(match = re.exec(html)) {
	      add(html.slice(cursor, match.index))(match[1], true);
	      cursor = match.index + match[0].length;
	  }
	  add(html.substr(cursor, html.length - cursor));
	  code += 'return r.join("");';
	  return new Function(code.replace(/[\r\t\n]/g, '')).apply(options);
	}
	// Share the template builder with express
	app.locals.template=template;

	//
	// open a shared map
	function get(req,res) {

	  var l=req.query.l, 
	      boatId=req.params.boatId,
	      curve=req.query.c, 
	      start=curveStartTimeStr(curve),
	      end=curveEndTimeStr(curve),
	      width=1024, height=536;

	  //
	  // build image url that keep ideal FB ratio 1.91:1 (600x315, 800x418, 1024x536)
	  var ogImg=[
	    "/api/map",boatId,l,start,end,width+"-"+height+".png"
	  ].join('/');

	  //
	  // Load current boat
	  mongoose.model('Boat').findById(boatId).select('name').exec(function(err,boat) {
	  	if(err){
	  		return res.status(500).send(err);
	  	}

	  	//
	  	// build shared url 
	  	var hostname=req.protocol + '://' + req.host;

	  	//
	  	// prepare social data model
	  	// TODO make this information more accurate
		  var model={
		    url:hostname+req.url,
		    title:'New session for '+boat.name,
		    description:"From "+new Date(start).toDateString(),
		    img:hostname+ogImg,
		    w:width,
		    h:height
		  }
		  res.send(req.app.locals.template(index,model));	
	  });
	}

	//
	// define router
	router.get('/:boatId',
	           auth.maybeAuthenticated(), 
	           access.boatReadAccess,
	           get);
	return router;
}
