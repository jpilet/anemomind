Pre-requisites 

For GKE
=> Install gcloud 
	=> configure gcloud 
		- gcloud init (login), select project & default zone
=> Install gsutils 


Build docker images 

For GKE - tag the images with appropriate project name. Note that the project name is unique across google cloud

docker build -t gcr.io/$PROJECT_NAME/anemoweb:latest -f src/server/Dockerfile .

docker build -t gcr.io/$PROJECT_NAME/anemoweb:latest -f www2/Dockerfile --build-arg CPP_DOCKER_IMAGE=gcr.io/$PROJECT_NAME/anemocpp:latest .

For GKE 

- push images to google cloud registry (private)

gcloud docker --authorize-only 

docker push <image_name>



Kubernetes Cluster 

- generate kubernetes cluster 
