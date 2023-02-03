# Anemomind Postgresql

Migrating from existing sqllite to postgressql.

## Unit Test

### Pre-requisites

1. postgresql must be up and running.
2. databases ep and newendpt must be created in postgresql
3. update the ```.env``` according to the postgresql configurations

### Test
```cd nodemodules/endpoint && npm install && ./node_modules/.bin/mocha```