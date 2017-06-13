# Distribution of VISE

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

## todo
continue with [this tutorial](https://docs.docker.com/get-started/part2/#recap-and-cheat-sheet-optional)

Abhishek Dutta  
02 June 2017


