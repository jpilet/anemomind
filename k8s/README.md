Pre-requisites 

For GKE
=> Install gcloud 
	=> configure gcloud 
		- gcloud init (login), select project & default zone
=> Install gsutils 


Build docker images 

For GKE - tag the images with appropriate project name. Note that the project name is unique across google cloud

    ./src/build_docker_prod.sh PROJECT_NAME <TAG>

For GKE 

- push images to google cloud registry (private)

gcloud docker --authorize-only 

docker push gcr.io/$PROJECT_NAME/anemomind_anemocppserver:latest
docker push gcr.io/$PROJECT_NAME/anemomind_anemowebapp:latest



Kubernetes Cluster 

- generate kubernetes cluster 
