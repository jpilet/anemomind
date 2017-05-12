#!/bin/bash
set -e
echo "Using NODE_ENV='${NODE_ENV}'"
uri=$(node catconfig.js %s x.mongo.uri)
echo "Using URI='${uri}'"
mongo $uri << 'EOF'
  db.sailingsessions.aggregate(
    {
      $group: {
        _id: "$boat",
        maxSpeedOverGround: { $max: "$maxSpeedOverGround" } 
      } 
    },
    { $sort : { maxSpeedOverGround : -1 } });
EOF
