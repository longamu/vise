# Code Documentation for Relja Retrival

## Query
```
src/v2/api/api_v2.cpp
    API API_obj( spatVerifObj, mq, dset );
    API_obj.server(io_service, APIport);

src/api/spatial_api.cpp
  class API : public absAPI { }
  API::getReply() { }
    API::queryExecute() { }
    API::processImage() { }
      spatialVerifV2_OBJECT->externalQuery_computeData(imageFn, queryObj);
      class spatialVerifV2 : public retrieverV2, public spatialRetriever { }
      src/v2/retrieval/spatial_verif_v2.cpp

      retrieverV2::externalQuery_computeData( std::string imageFn, query const &queryObj ) const { }
      /home/tlm/dev/vise/src/v2/retrieval/retriever_v2.cpp
        
src/api/abs_api.cpp
  absAPI::server(boost::asio::io_service& io_service, short int port)

```

All the functionality (search using an internal image, search using external image, 
return match regions, etc) are performed using `src/api/spatial_api.cpp`.

Abhishek Dutta  
30 April 2017
