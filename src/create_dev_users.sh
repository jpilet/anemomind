#!/bin/sh

MONGO_URL=${MONGO_URL:=mongodb://localhost:27017/anemomind-dev}

# Insert (or get) test user

set +e
testUserId=$(mongo --quiet ${MONGO_URL} --eval "print(db.users.find({email:'test@anemomind.com'})[0]._id + '')")
R=$?
set -e

echo "adding users:"
echo "- user: test@anemomind.com, password: anemoTest"
echo "- user: admin@anemomind.com, password: anemoTest"

if [ $R -ne 0 ]; then
  mongo --quiet ${MONGO_URL} --eval "db.users.insert({name:'Test User','provider' : 'local', 'name' : 'test', 'email' : 'test@anemomind.com', 'hashedPassword' : 'bj0zHvlC/YIzEFOU7nKwr+OHEzSzfdFA9PMmsPGnWITGHp1zlL+29oa049o6FvuR2ofd8wOx2nBc5e2n2FIIsg==', 'salt' : 'bGwuseqg/L/do6vLH2sPVA==', 'role' : 'user' });"
  testUserId=$(mongo --quiet ${MONGO_URL} --eval "print(db.users.find({email:'test@anemomind.com'})[0]._id + '')")

mongo --quiet ${MONGO_URL} --eval "db.users.insert({name:'Test Admin','provider' : 'local', 'name' : 'test admin', 'email' : 'admin@anemomind.com', 'hashedPassword' : 'bj0zHvlC/YIzEFOU7nKwr+OHEzSzfdFA9PMmsPGnWITGHp1zlL+29oa049o6FvuR2ofd8wOx2nBc5e2n2FIIsg==', 'salt' : 'bGwuseqg/L/do6vLH2sPVA==', 'role' : 'admin' });" || true
fi

adminUserId=$(mongo --quiet ${MONGO_URL} --eval "print(db.users.find({email:'admin@anemomind.com'})[0]._id + '')")

