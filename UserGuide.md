# VISE User Guide

This guide assumes that you have already [installed VISE](INSTALL.md). 

 1. Click `macos_start.command` script to start the VISE application (`linux_start.sh` if you are in Linux).

 2. A web browser window is automatically opened for the URL [http://localhost:9971](http://localhost:9971) as shown in [this screenshot](docs/help/vise/img/vise_home.png)

 3. Create a new search engine by typing in the search engine name and pressing the *Create* button as shown in [this screenshot](docs/help/vise/img/vise_training_create_search_engine.png)

 4. You will now be taken to the *Setting* page. Copy all your images -- that you wish to make search-able -- to `$HOME/vgg/mydata/images/` folder as shown in [this screenshot](docs/help/vise/img/vise_training_setting.png). Your images *must* be in this folder for VISE to work (you may need to create it when running for the first time). Before pressing the *Submit* button, ensure that all the images are copied correctly in the folder. For example, for MacOS users, this folder must be `/Users/abhishek/vgg/mydata/images/` while for Linux users, this folder can be `/home/abhishek/vgg/mydata/images/`.

 5. Now you will see an information page which contains the disk-space, memory and time required to make the images searchable as shown in [this screenshot](docs/help/vise/img/vise_training_info.png). If you are happy with these details, proceed ahead by pressing *Proceed*.

 6. The main process of making images searchable begins now. This process involves going through different stages: Preprocessing, Stage-1 ... Stage-5. as shown in these screenshots:
   * [Preprocessing](docs/help/vise/img/vise_training_preprocess.png)
   * [Stage-1](docs/help/vise/img/vise_training_stage1.png)
   * [Stage-4](docs/help/vise/img/vise_training_stage4.png)

 7. Once this process completed, you will be automatically forwarded to this URL [http://localhost:9971](http://localhost:9971) as shown in [this screenshot](docs/help/vise/img/vise_load_ox5k.png) -- where the list of images contain your images.
   * Click on any of the images shown in the list
   * Using mouse, define a rectangular region in the selected image (click and drag to draw a region)
   * Press *Search* button at the bottom of the page
   * Now, you will see a list of images ranked according to a score (higher score for better matching results)
   * Click *Detailed matches* to see the details of matched image
   * You can compare the query image region with region in search result using *Image Comparison* link at the bottom of this page

## Known Issues
 * Create search engine stops responding:
   * upgrade your VISE software to [vise-1.0.1](https://gitlab.com/vgg/vise/tags/vise-1.0.1)

 * [VISE web browser based user interface for indexing stops responding](https://gitlab.com/vgg/vise/issues/11)
   * upgrade your VISE software to [vise-1.0.1](https://gitlab.com/vgg/vise/tags/vise-1.0.1)
   * Press "Refresh" button (beside the "Log" button) on top to reload messages.

 * [VISE search engine interface breaks down with international character sets (non ASCII characters)](https://gitlab.com/vgg/vise/issues/15)
   * we are working to fix this issue
   * subscribe to [this issue](https://gitlab.com/vgg/vise/issues/15) to receive updates

Last updated: 06 July 2017  
Abhishek Dutta

