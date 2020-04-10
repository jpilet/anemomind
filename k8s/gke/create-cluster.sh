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

# setting vpc and subnet
echo 'Creating VPC...'
gcloud compute --project=${PROJECT_NAME} networks create ${ANEMO_VPC} --subnet-mode=custom && \
gcloud compute --project=${PROJECT_NAME} networks subnets create ${SUBNET} --network=${ANEMO_VPC} --region=${REGION} \
--range=10.0.0.0/9

echo 'Creating router'
gcloud compute --project=${PROJECT_NAME} routers create ${ROUTER} --asn=65534 --network=${ANEMO_VPC} --region=${REGION}

echo 'Creating subnet...'
gcloud compute firewall-rules create ${SN_RULE1} --network ${ANEMO_VPC} --allow tcp,udp,icmp \
--source-ranges 10.0.0.0/24 && \
gcloud compute firewall-rules create ${SN_RULE2} --network ${ANEMO_VPC} --allow tcp:22,tcp:3389,icmp

echo 'Creating kubernetes cluster ...'
gcloud beta container --project ${PROJECT_NAME} clusters create ${CLUSTER_NAME} --zone $ZONE --no-enable-basic-auth --cluster-version "1.13.11-gke.14" --machine-type=$MACHINE_TYPE --image-type=$IMAGE_TYPE --num-nodes "3" --enable-stackdriver-kubernetes --enable-private-nodes --master-ipv4-cidr "172.16.0.0/28" --enable-ip-alias --network ${ANEMO_VPC} --subnetwork ${SUBNET} --default-max-pods-per-node "110" --enable-master-authorized-networks --master-authorized-networks 0.0.0.0/0 --addons HorizontalPodAutoscaling,HttpLoadBalancing --enable-autoupgrade --enable-autorepair
echo 'checking cluster ...'
gcloud beta container clusters get-credentials ${CLUSTER_NAME}


echo 'configuring daemonset on the cluster nodes ...'
# Configure host VM using daemonset to disable hugepages
kubectl apply -f ${KUBE_RESOURCES_PATH}/mongo-hostvm-node-configurer-daemonset.yaml

# Define storage class for dynamically generated persistent volumes
# NOT USED IN THIS EXAMPLE AS EXPLICITLY CREATING DISKS FOR USE BY PERSISTENT
# VOLUMES, HENCE COMMENTED OUT BELOW
kubectl apply -f ./resources/mongo-gce-ssd-storageclass.yaml


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

sleep 100

# retriving ips of each mongo container
>temp.txt
for i in mongod-0 mongod-1 mongod-2
	do 
		a=`kubectl describe services $i | grep "10.0.*" | cut -d ":" -f2`
		if [ $i != "mongod-2" ]; then 
			echo -n "$a:27017,">>temp.txt 
		else 
			echo -n "$a:27017">>temp.txt 
		fi 
	done
mongo_ips=`cat temp.txt | tr -d ' '`
rm -f temp.txt

# Configure mongo users 
# Load default anemoming users 
echo "loading default data into mongodb ..."
/bin/sh load-mongo.sh $MONGO_PWD ${mongo_ips};

echo "mapping gcloud key as Kubernetes secret ..."
# create gcloud service account credentials
kubectl create secret generic gcs-key --from-file=key.json=$GCLOUD_CREDS_KEY

sleep 30

echo "deploying anemo web application ..."
# Create node web app deployment
envsubst < ${KUBE_RESOURCES_PATH}/anemoweb.yaml > /tmp/anemoweb.yaml

# Replace existing MONGO_URI with new IPs
sed -i "s/@[a-z]*-[0-9]*.*anemomind-dev/@$mongo_ips\/anemomind-dev/g" /tmp/anemoweb.yaml

kubectl apply -f /tmp/anemoweb.yaml
echo

sleep 30

# Print current deployment state
kubectl get persistentvolumes
echo
kubectl get all 
