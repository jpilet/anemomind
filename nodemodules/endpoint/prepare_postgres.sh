#!/bin/bash
set -e

SEARCH_PATH=/opt/local/lib/postgresql12/bin/:$PATH
INITDB="$(PATH=$SEARCH_PATH which initdb)"
PSQL="$(PATH=$SEARCH_PATH which psql)"
PG_CTL="$(PATH=$SEARCH_PATH which pg_ctl)"

DB_PATH="$(pwd)/pg_test_db"
rm -fR "${DB_PATH}"
mkdir -p "${DB_PATH}"
(cd pg_test_db ; "${INITDB}" -D "${DB_PATH}" -U $(whoami) )

"${PG_CTL}" -D "${DB_PATH}" start

echo "CREATE DATABASE endpoint;" | "${PSQL}" postgres 

echo "Database created. Shutdown with:"
echo "${PG_CTL}" -D "${DB_PATH}" stop
echo
echo "Run tests with:"
echo "env DATABASE_URL=endpoint ./node_modules/.bin/mocha  test/endpoint.postgres.js"
