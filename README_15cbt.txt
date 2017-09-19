Save annotations to : /home/tlm/vgg/vise/search_engines/15c_bt/training_data/image_annotations.csv

$ make -j8 && ./vise /home/tlm/dev/vise/ /home/tlm/vgg/vise/

## Source code

/home/tlm/dev/vise/src/v2/api/api_v2.cpp
    // ImageMetadata
    boost::filesystem::path metadata_fn("/home/tlm/vgg/vise/search_engines/15c_bt/training_data/image_annotations.csv");
    boost::filesystem::path preprocess_fn("/home/tlm/vgg/vise/search_engines/15c_bt/training_data/preprocess_log.csv");
    ImageMetadata::Instance()->LoadMetadata( metadata_fn );
    ImageMetadata::Instance()->LoadPreprocessData( preprocess_fn );

/home/tlm/dev/vise/src/api/ImageMetadata.cc
/home/tlm/dev/vise/src/api/spatial_api.cpp
/home/tlm/dev/vise/src/api/abs_api.cpp

/home/tlm/dev/vise/src/ui/web/do_search.py
