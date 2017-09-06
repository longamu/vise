# VISE Installation Instructions for Windows 10

### Step 1 : Install Docker
 Download [Docker-CE for Windows](https://store.docker.com/editions/community/docker-ce-desktop-windows) 
 and install the docker application just like you install any other Windows 
 application. The [installation instructions](https://docs.docker.com/docker-for-windows/) 
 guide at docker website provides more details about the installation process.
 
 The VISE docker image has been built using **docker version 17.06.0**. 
 Please ensure that you install the same (or superior) version of docker in your 
 computer.
 
 The VISE application requires access to your local disk to store application 
 data and load the training images. Open docker settings and in "Shared Drives" 
 menu, select "C" (or any other drive) as shown below:
 
 <img src="docs/help/kitematic/windows/Docker_settings_shared_drive.png" width="300"/>
 
### Step 2 : Install and Open Kitematic
 [Kitematic](https://kitematic.com/) is a tool that provides a graphical user 
 interface to load, start, stop and configure docker containers. [Download and 
 install](https://www.docker.com/products/docker-toolbox) this tool.
 
 Once this tool is installed, double click the Kitematic icon in Desktop to run 
 this tool.

### Step 3 : Install oxvgg/vise
 In the Kitematic tool, enter the following search keyword: **oxvgg**. This will 
 show all the docker containers released by the Visual Geometry Group (VGG) as 
 shown below:
 
 <img src="docs/help/kitematic/windows/Kitematic_search_oxvgg.png" width="400"/>
 
 Click **Create** button in the bottom right corner of the box panel corresponding 
 to VISE. This will start download of the VISE container (may take few minutes).
 
 <img src="docs/help/kitematic/windows/Kitematic_downloading_image_vise.png" width="400"/>
 
 
 
### Step 4 : Configure ports and volume
### Step 5 : Run oxvgg/vise

Once the installation is complete, please follow the [User Guide](UserGuide.md) 
to train VISE for searching your own image collections.
