# To suppress the output: --quiet
mongo anemomind-dev --eval "db.users.insert({name:'Test User','provider' : 'local', 'name' : 'test', 'email' : 'test@anemomind.com', 'hashedPassword' : 'bj0zHvlC/YIzEFOU7nKwr+OHEzSzfdFA9PMmsPGnWITGHp1zlL+29oa049o6FvuR2ofd8wOx2nBc5e2n2FIIsg==', 'salt' : 'bGwuseqg/L/do6vLH2sPVA==', 'role' : 'user' });"

# HOW CAN WE INSERT A BOAT WITH A SPECIFIC ID?
mongo anemomind-dev --eval "db.boats.insert({_id: '001'});"
# The password for the registered user above is 'anemoTest'.

node sync2http.js
