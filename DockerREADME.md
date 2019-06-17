Docker configuration for production and development environment.

In Production environment following modules have been containerized in docker: 
 
  1. C++ log processing server named as **_anemomind_anemocppserver_1_**  
  2. Anemolab web app named as **_anemomind_anemowebapp_1_**  
  3. Mongodb named as **_anemomongo_**  

In Devlopment Environment:
 
 1. C++ log processing server has been containerized in docker as **_anemomind_anemocppserver_1_**.
 
 2. On host machine user can make changes on cpp server code using VS Code IDE and those changes will reflected on container anemomind_anemocppserver_1 also user can build the code on container through host machine.
 

## Pre-requisites

Following packages must be installed:

1. _[Docker](https://docs.docker.com/install/)_
2. _[docker-compose](https://docs.docker.com/compose/install/)_
3. _[VS Code](https://code.visualstudio.com/download) (vs code is needed only for development evnvironment)_  


## Installation

 Containerized anemomind can be created through ```run.sh``` script.

 ```run.sh``` takes 1 argument as environment and this argument must be either ```prod``` or ```dev```
 
 To create containerized anemomind in **_Production_** use:
  ```
  ./run.sh prod
  ```
 And in **_Development_** use:
 ```
 ./run.sh dev
 ``` 

NOTE: In both the environments run.sh will compile and build the code base and hence might take a while running it for the first time.

## Usage

#### In Production

To check that containers are running type following command on the local terminal:  
```
docker container ls 
```   
if containers are running you will see follwing listing  

```
CONTAINER ID        IMAGE                      COMMAND                  CREATED             STATUS              PORTS                                 NAMES  
39de31712ccb        anemomind_anemowebapp      "grunt serve:dev --f…"   3 hours ago         Up 3 hours          0.0.0.0:9000->9000/tcp                anemomind_anemowebapp_1
7b0726d5d264        anemomind_anemocppserver   "/bin/sh -c 'cron &&…"   3 hours ago         Up 3 hours                                                anemomind_anemocppserver_1
461f4fb00d7a        mvertes/alpine-mongo       "/root/run.sh mongod…"   3 hours ago         Up 3 hours          0.0.0.0:27017->27017/tcp, 28017/tcp   anemomongo  
(NOTE: container IDs will be different)
```
To interact and to examine the contents of the containers type following commands on local machine:  

 ```
 docker exec -it {container_name/container_id} bash 
 ``` 
Example: 
 ```
 docker exec -it anemomind_anemowebapp_1 bash (use sh instead of bash in case of anemomind container)
 ``` 

   NOTE: All containers related logs will appear on terminal from where run.sh executed.

   To check logs of anemomind_anemocppserver_1 run:
   ```
   docker exec -it anemomind_anemocppserver_1 bash
   tail -f /var/log/cron.log
   ```
   To check cron job running on anemomind_anemocppserver_1 run:
   ```
   docker exec -it anemomind_anemocppserver_1 bash
   crontab -e (OR) cat /etc/cron.d/anemo-cron
   ```

#### In Development

 To check that container is running type following command on the local terminal:  
 ```
  docker container ls 
 ```   
 if containers are running you will see follwing listing
 ```
 CONTAINER ID        IMAGE                      COMMAND                  CREATED             STATUS              PORTS                                 NAMES  
 72d8b9d3c27a        anemomind_anemocppserver   "/bin/sh -c 'cron &&…"   3 hours ago         Up 3 hours                                                anemomind_anemocppserver_1
 NOTE: container ID will be different)
 ```
Steps to build Cpp Server on **_anemomind_anemocppserver_1_** container:

1. set **_root password_** of container:
   
   a. login to anemomind_anemocppserver_1 as:
   ```
    docker exec -it anemomind_anemocppserver_1 bash
   ```
   b. In anemomind_anemocppserver_1 container type: ```passwd```  

   NOTE: user can also use key-based authentication by copying security keys to container as:  

   ```
   ssh-copy-id -p 7188 root@localhost
   ```


2. On host machine:  
   a. Open anemomind project in VS Code IDE, make required changes on cpp server code and save the changes.  
   b. Press ```ctrl+shift+B```, a popup with name ```build``` will appear, click on the popup.  
   c. Accept the keys and then enter the root password of **_anemomind_anemocppserver_1_** on VS Code Terminal to start the build process.  

