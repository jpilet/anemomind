// For chart tile random access and streaming for export
db.charttiles.createIndex({boat: 1, zoom: 1, tileno:1, what:1, source: 1});

db.perfstats.createIndex({boat: 1, urlName: 1}, { unique: true } );
