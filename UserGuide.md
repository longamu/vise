# VISE User Guide

This guide assumes that you have already [installed VISE](INSTALL.md) and it is running. 

 1. Open [http://localhost:9971](http://localhost:9971) in a web browser as shown in [this screenshot](docs/help/vise/img/vise_home.png)
   * **Do not open `localhost:9971` in multiple browser windows**. Just use a single browser window
   * Once you have trained a search engine or loaded a search engine, **you cannot load or train another search engine unless you restart VISE through Kitematic** and revisit [http://localhost:9971](http://localhost:9971).

 2. Create a new search engine by typing in the search engine name and pressing the *Create* button as shown in [this screenshot](docs/help/vise/img/vise_training_create_search_engine.png)

 3. You will now be taken to the *Setting* page. Copy all your images to the local folder 
that maps to the docker folder `/opt/ox/vgg/mydata/images`. For MacOS and 
Windows users, this folder is the local folder that was mapped to 
`/opt/ox/vgg/mydata/images` as described in *Step 4* of the installation guide for 
[MacOS](INSTALL-MacOS.md) and [Windows](INSTALL-Windows.md).
   
 4. Now you will see an information page which contains the disk-space, memory and time required to make the images searchable as shown in [this screenshot](docs/help/vise/img/vise_training_info.png). If you are happy with these details, proceed ahead by pressing *Proceed*.

 5. The main process of making images searchable begins now. This process involves going through different stages: Preprocessing, Stage-1 ... Stage-5. as shown in these screenshots:
   * [Preprocessing](docs/help/vise/img/vise_training_preprocess.png)
   * [Stage-1](docs/help/vise/img/vise_training_stage1.png)
   * [Stage-4](docs/help/vise/img/vise_training_stage4.png)

 6. Once this process completed, you will be automatically forwarded to this URL [http://localhost:9971](http://localhost:9971) as shown in [this screenshot](docs/help/vise/img/vise_load_ox5k.png) -- where the list of images contain your images.
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

Last updated: 06 Sep. 2017  
Abhishek Dutta

