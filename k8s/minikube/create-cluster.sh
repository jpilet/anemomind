#!/bin/bash
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
BASEDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
KUBE_RESOURCES_PATH="${BASEDIR}/resources"


TMPFILE=$(mktemp)
/usr/bin/openssl rand -base64 741 > $TMPFILE
kubectl create secret generic shared-bootstrap-data --from-file=internal-auth-mongodb-keyfile=$TMPFILE
rm $TMPFILE


# Create mongodb service with mongod stateful-set
# TODO: Temporarily added no-valudate due to k8s 1.8 bug: https://github.com/kubernetes/kubernetes/issues/53309
kubectl apply -f $KUBE_RESOURCES_PATH/mongodb-minikube.yaml --validate=false
sleep 5

echo "deploying cppserver ..."
# Create cppserver deployment
kubectl apply -f ${KUBE_RESOURCES_PATH}/anemocpp.yaml
echo

echo "deploying anemo web application ..."
# Create node web app deployment
kubectl apply -f ${KUBE_RESOURCES_PATH}/anemoweb.yaml
echo

sleep 30

# Print current deployment state (unlikely to be finished yet)
kubectl get all 
kubectl get persistentvolumes
echo
echo "Keep running the following command until all pods are shown as running:  kubectl get all"
echo


