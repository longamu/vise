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
    ...

src/api/abs_api.cpp
  absAPI::server(boost::asio::io_service& io_service, short int port)

```

Abhishek Dutta  
30 April 2017
