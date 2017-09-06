# Installation in Linux

### Step 1: Install docker
These docker installation instructions apply to Ubuntu 16.04 (xenial) release. 
For other releases, please refer to instructions at [docker website](https://docs.docker.com/engine/installation/linux/ubuntu/#install-using-the-repository)
```
sudo apt-get remove docker docker-engine
sudo apt autoremove
sudo apt-get install apt-transport-https ca-certificates curl software-properties-common
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
sudo apt-key fingerprint 0EBFCD88
sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"
sudo apt-get update
apt-cache madison docker-ce # to show the list of available versions
sudo apt-get install docker-ce=17.06.2~ce-0~ubuntu
```

### Step 2: Download VISE docker images
```
sudo docker pull oxvgg/vise
```

### Step 3: Run VISE container
To run the VISE container, you have to map two ports (9971 and 9973) and the following two volumes:
 * `/opt/ox/vgg/vise/application_data` : This is the folder where VISE stores all its internal application files
 * `/opt/ox/vgg/mydata/images` : This is the folder where user will be asked to copy their personal images that needs to be indexed and made searchable

For the purpose of illustration, I have mapped these two folders to `$HOME/vise/application_data` 
and `$HOME/vise/training_images/` respectively.

```
sudo docker run -p 9971:9971 -p 9973:9973 -v $HOME/vise/application_data:/opt/ox/vgg/vise/application_data/ -v $HOME/vise/training_images/:/opt/ox/vgg/mydata/images oxvgg/vise
```

Now visit [http://localhost:9971](http://localhost:9971) in you web browser and follow 
the [User Guide](UserGuide.md) to train VISE for searching your own image collections.

