

Following modules have been containerized in docker  
  1. C++ log processing server named as anemomind_anemocppserver_1  
  2. Anemolab web app named as anemomind_anemowebapp_1  
  3. Mongodb named as anemomongo  

## Pre-requisites

Docker and docker-compose must be installed.  
Installation links: [docker](https://docs.docker.com/install/) [dokcer-compose](https://docs.docker.com/compose/install/)


## Usage

In root directory of project run: 
```
docker-compose up
```  

docker-compose up will compile and build the code base and hence might take a while running it the first time.  

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
example : docker exec -it anemomind_anemowebapp_1 bash (use sh instead of bash in case of anemomind container)
``` 

   NOTE: All containers related logs will appear on terminal from where docker-compose up command executed.

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