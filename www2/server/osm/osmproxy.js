'use strict';

var fs = require('fs');

var express = require('express');
var mkdirp = require('mkdirp');
var request = require('request');
var router = express.Router();

router.get('/:scale/:x/:y.png', function(req, res) {
  let localImagePath = config.root
    + (config.env == 'production' ? '/public' : '/client')
    + '/osm/';

  let folder = `${localImagePath}/${req.params.scale}/${req.params.x}`;
  let localPath =`${folder}/${req.params.y}.png`; 

  fs.readFile(localPath, function(err, data) {
    console.log('reading file: ' + localPath + ': ' + err);
    if (err) {
      // try to download
      let s =  'a';
      let url = "http://stamen-tiles-" + s + ".a.ssl.fastly.net/toner-lite/"
            + req.params.scale + "/" + req.params.x + "/" + req.params.y + ".png";
      request.get({encoding: null, url: url}, function (err, resp) {
        if (!err && resp.statusCode == 200) {
          res.type('image/png');
          res.status(200).send(resp.body);
          mkdirp(folder, function (err) {
            if (err) {
              console.error(err);
            } else {
              fs.writeFile(localPath, resp.body, 'binary', (err) => {
                if (err) {
                  console.log('Error while saving to ' + localPath
                              + ': ' + err);
                }
              });
            }
          });
        } else {
          res.status(404);
        }
      });
    } else {
      res.type('image/png');
      res.status(200).send(data);
    }
  });
});

module.exports = router;

