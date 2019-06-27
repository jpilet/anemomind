#!/bin/sh
##
# Script to deploy anemomind Kubernetes cluster  with the following [To be replaced with Terraform/Skaffold in later releases]
#  => 3 Node GKE cluster backed up by GCE nodes specified by --machine-type and --image-type for given project name/id
#  => GCE SSD persistent disks per mongodb stateful pod
#  => Create GCS storage bucket
#  => Mongodb cluster with primary,secondary and arbiter - Statefulset running the mongodb replicaset - 3 pods, 1 headless service
#  => kubernetes secret deployment for service account access to CPP & anemoweb pods
#  => CPP server/processor kubernetes deployment running the cron job - single pod
#  => 'N' node scalable anemoweb pods plus Loadbalancer service
##

# Load up .env
export $(egrep -v '^#' .env | xargs)

# Create new GKE Kubernetes cluster (using host node VM images based on Ubuntu (Dev Cluster)


echo 'Authorizing to gcloud using service account ...'
gcloud auth activate-service-account --key-file=$GCLOUD_CREDS_KEY

gcloud config set project $PROJECT_NAME
gcloud config set compute/zone $ZONE

echo 'Creating kubernetes cluster ...'
gcloud container clusters create ${CLUSTER_NAME} --image-type=$IMAGE_TYPE --machine-type=$MACHINE_TYPE --preemptible

echo 'checking cluster ...'
gcloud container clusters get-credentials ${CLUSTER_NAME}


echo 'configuring daemonset on the cluster nodes ...'
# Configure host VM using daemonset to disable hugepages
kubectl apply -f ${KUBE_RESOURCES_PATH}/mongo-hostvm-node-configurer-daemonset.yaml

# Define storage class for dynamically generated persistent volumes
# NOT USED IN THIS EXAMPLE AS EXPLICITLY CREATING DISKS FOR USE BY PERSISTENT
# VOLUMES, HENCE COMMENTED OUT BELOW
#kubectl apply -f ../../resources/mongo-gce-ssd-storageclass.yaml

# Register GCE Fast SSD persistent disks and then create the persistent disks 
echo "Creating GCE disks"
for i in 1 2 3
do
    gcloud --quiet compute disks create --zone $ZONE --size 10GB --type pd-ssd pd-ssd-disk-$i
done
sleep 3

# Create persistent volumes using disks created above
echo "Creating GKE Persistent Volumes"
for i in 1 2 3
do
    sed -e "s/INST/${i}/g" ${KUBE_RESOURCES_PATH}/mongo-xfs-gce-ssd-persistentvolume.yaml > /tmp/mongo-xfs-gce-ssd-persistentvolume.yaml
    kubectl apply -f /tmp/mongo-xfs-gce-ssd-persistentvolume.yaml
done
rm /tmp/mongo-xfs-gce-ssd-persistentvolume.yaml
sleep 3

# Create keyfile for the MongoD cluster as a Kubernetes shared secret
TMPFILE=$(mktemp)
/usr/bin/openssl rand -base64 741 > $TMPFILE
kubectl create secret generic shared-bootstrap-data --from-file=internal-auth-mongodb-keyfile=$TMPFILE
rm $TMPFILE

# Create mongodb service with mongod stateful-set
kubectl apply -f ${KUBE_RESOURCES_PATH}/mongodb-gke.yaml
echo

# Wait until the final (3rd) mongod has started properly
echo "Waiting for the 3 containers to come up (`date`)..."
echo " (IGNORE any reported not found & connection errors)"
sleep 30
echo -n "  "
until kubectl --v=0 exec mongod-2 -c mongod-container -- mongo --quiet --eval 'db.getMongo()'; do
    sleep 5
    echo -n "  "
done
echo "...mongod containers are now running (`date`)"
echo

# Configure mongo users 
# Load default anemoming users 
echo "loading default data into mongodb ..."
/bin/sh load-mongo.sh $MONGO_PWD;

echo "mapping gcloud key as Kubernetes secret ..."
# create gcloud service account credentials
kubectl create secret generic gcs-key --from-file=key.json=$GCLOUD_CREDS_KEY

echo "deploying cppserver ..."
# Create cppserver deployment
envsubst < ${KUBE_RESOURCES_PATH}/anemocpp.yaml > /tmp/anemocpp.yaml
kubectl apply -f /tmp/anemocpp.yaml
echo

echo "deploying anemo web application ..."
# Create node web app deployment
envsubst < ${KUBE_RESOURCES_PATH}/anemoweb.yaml > /tmp/anemoweb.yaml
kubectl apply -f /tmp/anemoweb.yaml
echo

sleep 30

# Print current deployment state
kubectl get persistentvolumes
echo
kubectl get all 

