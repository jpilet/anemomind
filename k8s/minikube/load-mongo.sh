#!/bin/bash
##
# Script to connect to the first Mongod instance running in a container of the
# Kubernetes StatefulSet, via the Mongo Shell, to initalise a MongoDB Replica
# => Set and create a MongoDB admin user.
# => Create Anemomind users test & admin
#
##

# Check for password argument
if [[ $# -eq 0 ]] ; then
    echo 'You must provide one argument for the password of the "main_admin" user to be created'
    echo '  Usage:  load-mongo.sh yourpwd'
    echo
    exit 1
fi

# Initiate MongoDB Replica Set configuration
echo "Configuring the MongoDB Replica Set"
kubectl exec mongod-0 -c mongod-container -- mongo --eval 'rs.initiate({_id: "MainRepSet", version: 1, members: [ {_id: 0, host: "mongod-0.mongodb-service.default.svc.cluster.local:27017"}, {_id: 1, host: "mongod-1.mongodb-service.default.svc.cluster.local:27017"}, {_id: 2, host: "mongod-2.mongodb-service.default.svc.cluster.local:27017"} ]});'
echo

# Wait for the MongoDB Replica Set to have a primary ready
echo "Waiting for the MongoDB Replica Set to initialise..."
kubectl exec mongod-0 -c mongod-container -- mongo --eval 'while (rs.status().hasOwnProperty("myState") && rs.status().myState != 1) { print("."); sleep(1000); };'
#sleep 2 # Just a little more sleep to ensure everything is ready!
sleep 20 # More sleep to ensure everything is ready! (3.6.0 workaround for https://jira.mongodb.org/browse/SERVER-31916 )
echo "...initialisation of MongoDB Replica Set completed"
echo

# Create the admin user (this will automatically disable the localhost exception)
echo "Creating user: 'main_admin'"
kubectl exec mongod-0 -c mongod-container -- mongo --eval 'db.getSiblingDB("admin").createUser({user:"main_admin",pwd:"'"${1}"'",roles:[{role:"root",db:"admin"}]});'
echo

# Create the test user for anemomind-dev 
echo "Creating user: 'test' for anemomind-dev"
kubectl exec mongod-0 -c mongod-container -- mongo anemomind-dev -u main_admin -p ${1} --authenticationDatabase admin --eval "db.users.insert({name:'Test User','provider' : 'local', 'name' : 'test', 'email' : 'test@anemomind.com', 'hashedPassword' : 'bj0zHvlC/YIzEFOU7nKwr+OHEzSzfdFA9PMmsPGnWITGHp1zlL+29oa049o6FvuR2ofd8wOx2nBc5e2n2FIIsg==', 'salt' : 'bGwuseqg/L/do6vLH2sPVA==', 'role' : 'user' });"
echo

# Create the test user for anemomind-dev 
echo "Creating user: 'admin' for anemomind-dev"
kubectl exec mongod-0 -c mongod-container -- mongo anemomind-dev -u main_admin -p ${1} --authenticationDatabase admin --eval "db.users.insert({name:'Test Admin','provider' : 'local', 'name' : 'test admin', 'email' : 'admin@anemomind.com', 'hashedPassword' : 'bj0zHvlC/YIzEFOU7nKwr+OHEzSzfdFA9PMmsPGnWITGHp1zlL+29oa049o6FvuR2ofd8wOx2nBc5e2n2FIIsg==', 'salt' : 'bGwuseqg/L/do6vLH2sPVA==', 'role' : 'admin' });"
echo

