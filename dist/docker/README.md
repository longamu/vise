# Docker image of VGG Image Search Engine (VISE)

Assuming that docker has already been installed on a user's computer, the aim 
of this docker image is to provide a one click installation of the VGG Image 
Search Engine (VISE).

## Install Docker
Reference: [docker-installation-guide](https://docs.docker.com/engine/installation/linux/ubuntu/#install-using-the-repository)

```
sudo apt-get remove docker docker-engine
sudo apt autoremove
sudo apt-get install     apt-transport-https     ca-certificates     curl     software-properties-common
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
sudo apt-key fingerprint 0EBFCD88
sudo add-apt-repository    "deb [arch=amd64] https://download.docker.com/linux/ubuntu (lsb_release -cs) stable"
sudo apt-get update
apt-cache madison docker-ce
sudo apt-get install docker-ce=17.03.0~ce-0~ubuntu-xenial
```

## Building VISE image
 * "Dockerfile will define what goes on in the environment inside your container". 
 * continue with [this tutorial](https://docs.docker.com/get-started/part2/#recap-and-cheat-sheet-optional)

```
cd /home/tlm/dev/vise/dist/docker
ls
  Dockerfile  README.md

sudo docker build --no-cache=true -t vise .  # build the VISE image
sudo docker images -a
sudo docker run vise
sudo docker run -d vise                      # run VISE image in background
sudo docker attach vise

sudo docker ps -a                            # list all running container
sudo docker rm 844694571ca0

sudo docker run -it vise bash                # run bash shell in interactive mode
sudo docker run -p 8080:8080 -it vise bash
sudo docker rm `sudo docker ps -a -q`
sudo docker rmi `sudo docker images -a -q` -f
```

Abhishek Dutta  
05 June 2017
