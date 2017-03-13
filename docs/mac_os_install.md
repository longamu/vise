* Environment
```
brew update
brew install wget
brew install python boost protobuf
echo 'import site; site.addsitedir("/usr/local/lib/python2.7/site-packages")' >> /Users/adutta/Library/Python/2.7/lib/python/site-packages/homebrew.pth
brew install imagemagick
brew link libpng
brew link --overwrite libpng
```

* Backend
  * Install all dependencies to a custom folder: $RR_DEP=/Volumes/Data2/adutta/rr_dependencies/

  * Update src/CMakeLists.txt as follows:
```
## Modifications to port relja_retrival to Mac OS
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../forbuild/")
list(APPEND CMAKE_PREFIX_PATH "/Volumes/Data2/adutta/rr_dependencies/fastann" "/Volumes/Data2/adutta/rr_dependencies/protobuf-2.6.1")
#set(BOOST_ROOT "/Volumes/Data2/adutta/rr_dependencies/boost_1_63_0/")
#set(Boost_NO_SYSTEM_PATHS "TRUE")
set(ENV{PKG_CONFIG_PATH} "/Volumes/Data2/adutta/rr_dependencies/ImageMagick-6.9.7-9/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
```

  * fastann :
```
# remove yasm dependency from relja_code/fastann/src/CMakeLists.txt
#ADD_LIBRARY(fastann SHARED dist_l2.cpp fastann.cpp randomkit.c ${YASM_OBJS})
ADD_LIBRARY(fastann SHARED dist_l2.cpp fastann.cpp randomkit.c)

// compile fastann
$ cd /Volumes/Data2/adutta/rr_dependencies/fastann
$ cmake /Volumes/Data2/adutta/relja_arandjelovic/relja_code/fastann/src
```

  * Install ImageMagick < 7.0.2 because cmake can only find version in 6.?.?
```
wget https://www.imagemagick.org/download/ImageMagick-6.9.7-9.tar.bz2
tar -zxvf ImageMagick-6.9.7-9.tar.bz2 
cd ImageMagick-6.9.7-9
CC=gcc-6 CXX=g++-6 ./configure --prefix=/Volumes/Data2/adutta/rr_dependencies/ImageMagick-6.9.7-9 --with-quantum-depth=16 --disable-dependency-tracking --with-x=yes --x-includes=/usr/X11R6/include --x-libraries=/usr/X11R6/lib/ --without-perl
make -j8
sudo make install
``` 

  * Compile boost
```
$ wget https://netcologne.dl.sourceforge.net/project/boost/boost/1.63.0/boost_1_63_0.tar.bz2

$ ./bootstrap.sh --prefix=/Volumes/Data2/adutta/rr_dependencies/boost_1_63_0 --with-toolset=gcc --with-libraries=filesystem,system,thread,date_time,chrono,atomic 

// update project-config.jam file to have the following
//using gcc : 6.3.0 : g++-6 ;

$ ./b2 variant=release threading=multi install
```

  * Compile google protobuf 2.6.1 (NOT NEEDED - `brew install boost` works well)
```
$ CC=gcc-6 CXX=g++-6 ./configure --prefix=/Volumes/Data2/adutta/rr_dependencies/protobuf-2.6.1
$ make && make install
```

  * Update src/util/macros.h
```
// added by @Abhishek to support compilation in Mac
#ifdef __APPLE__
    #define off64_t off_t
    #define fopen64 fopen
    #define ftello64 ftello
    #define fseeko64 fseeko
    #define pread64 pread

    typedef unsigned int uint;
#endif

// add space between string literal [ " (" ] and macro [ __FILE__ ]
#define ASSERT(expression) if (!(expression)) { std::cerr << "ASSERT failed: " #expression " in "  << __FUNCTION__ << " (" __FILE__ ":" << __LINE__ << ")\n"; exit(1); }
```

  * external/KMCode_relja/gauss_iir/gauss_iir.h
```
//#include <malloc.h> // the malloc of stdlib is used
```


  * Change order of include statements in /src/external/KMCode_relja/descriptor/corner.h
```
#include "../gauss_iir/gauss_iir.h"
#include "../ImageContent/imageContent.h"
```

  * Comment <malloc.h> in /src/external/KMCode_relja/homohraphy/homography.h
```
//#include <malloc.h>
```

  * Undefined "uint" error in src/external/KMCode_relja/ImageContent/ImageContent.{cpp,h}
```
// add this to src/external/KMCode_relja/ImageContent/ImageContent.h
// added by @Abhishek to support compilation in Mac
#define png_voidp_NULL    (png_voidp)NULL
#define png_infopp_NULL   (png_infopp)NULL

#define png_set_gray_1_2_4_to_8(arg) png_set_expand_gray_1_2_4_to_8(arg)

typedef unsigned int uint;
```

  * Update the external variable in src/external/KMCode_relja/descriptor/CornerDescriptor.{cpp,h}
```
//extern float scale_mult;
// added by @Abhishek to support compilation in Mac
float scale_mult = 3.0;
```
see this for more details: http://stackoverflow.com/questions/875655/linking-extern-variables-in-c

  * Update "src/preprocessing/CMakeLists.txt"
```
add_library( colour_sift colour_sift.cpp )
target_link_libraries( colour_sift
    image_util
    feat_getter    # added by @Abhishek to support compilation in Mac
    ${Boost_LIBRARIES} )
. . .
add_library( holidays_public holidays_public.cpp )
target_link_libraries( holidays_public
    image_util
    feat_getter    # added by @Abhishek to support compilation in Mac
    ${Boost_LIBRARIES} )
```

  * Undefined symbol for boost functions
```
// update "src/preprocessing/CMakeLists.txt"
target_link_libraries( feat_getter ellipse ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES})

// update "src/retrival/CMakeLists.txt"
target_link_libraries( multi_query retriever ${Boost_LIBRARIES})
```

  * Error cause by google protobuf "/usr/local/include/google/protobuf/stubs/atomicops.h"
```
// switch to protobuf-2.6.1
```

  * error in "src/v2/indexing/proto_db_header.proto"
```
// old: target_link_libraries( index_entry.pb ${PROTOBUF_LIBRARIES} )
target_link_libraries( proto_db_header.pb ${PROTOBUF_LIBRARIES} ) # added by @Abhishek to support compilation in Mac
```

  * Undefined symbol in "src/v2/indexing/uniq_entries.h"
```
// update src/v2/indexing/CMakeLists.txt
add_library( uniq_entries uniq_entries.cpp )
target_link_libraries( uniq_entries index_entry.pb ${Boost_LIBRARIES} ) # added by @Abhishek to support compilation in Mac
```

  * Undefined symbol "sameRandomUint32" in "src/v2/indexing/train/train_descs.{cpp,h}"
```
// update src/v2/indexing/train/CMakeLists.txt
target_link_libraries( train_descs
    clst_centres
    par_queue
    same_random # added by @Abhishek to support compilation in Mac
    image_util # added by @Abhishek to support compilation in Mac
    ${fastann_LIBRARIES}
    ${Boost_LIBRARIES} )

target_link_libraries( train_assign
    clst_centres # added by @Abhishek to support compilation in Mac
    ${fastann_LIBRARIES} # added by @Abhishek to support compilation in Mac
    ${Boost_LIBRARIES} )

target_link_libraries( train_hamming
    hamming_data.pb # added by @Abhishek to support compilation in Mac
    ${Boost_LIBRARIES} )
```

  * "Segmentation fault: 11"
```
void absAPI::session( socket_ptr sock ){
	. . .
	sock->close(error);
	. . .
}
```

  * Final compilation
```
cd /Volumes/Data2/adutta/relja_arandjelovic/relja_code/relja_retrieval
mkdir build_mac && cd build_mac
rm -fr * && CC=gcc-6 CXX=g++-6 cmake ../src
make -j8 api_v2
v2/api/api_v2 35280 ox5k
```

* Frontend

  * Resolve Image import from PIL
```
// update src/ui/web/upload.py
from PIL import Image;

// update src/ui/web/dynamic_image.py
from PIL import Image;
from PIL import ImageDraw;

// update src/ui/web/search_page.py
from PIL import Image;

// update src/ui/web/details.py
from PIL import Image;
from PIL import ImageDraw, ImageFont;

// update src/ui/web/register_images.py
from PIL import Image;

// update src/ui/web/web_api.py
from PIL import Image;
```

 * Frontend execution
```
$ python webserver.py 8080 ox5k 35280
```


## jp_draw
```
$ brew install cairo
$ cd src/external/jp_draw
$ Update setup.py: include_dirs=inc_dirs+['/usr/local/include/cairo/'],
$ python setup.py build
$ sudo python setup.py install
```

## Notes
  * To Debug segmentation fault using gdb:
```
$ gdb -ex=run --args v2/api/api_v2 35280 ox5k
```

  * Apple uses clang compiler by default. We need to switch to gcc based compiler.
```
brew install gcc@4.8
rm -fr * && CC=gcc-4.9 CXX=g++-4.9 cmake ../src/
```
  * Important note: http://stackoverflow.com/questions/18139710/using-c11-in-macos-x-and-compiled-boost-libraries-conundrum
```
After a lot of research, I learned the following:

Everything on MacOS X is built using stdlibc++ (including everything you install with homebrew -- which was my case)
stdlibc++ that comes with gcc 4.2 does not implement many of the features of c++11. Examples are unordered_map, to_string, stoi, ... only libc++ does.
libc++ is installed in MacOS by XCode.
Conclusion:

If you're using c++11, you have to use -stdlib=libc++.
If you want to use boost or any other library, you have to recompile them with -stdlib=libc++ as well.

If you use Homebrew, you can do it with and it will do the right thing for you:

brew install boost --c++11
```


<internalQuery><docID>50</docID><xl>5</xl><xu>100</xu><yl>30</yl><yu>400</yu><numberToReturn>100</numberToReturn></internalQuery> $END$
<internalQuery><docID>50</docID><xl>5</xl><xu>100</xu><yl>30</yl><yu>400</yu><numberToReturn>100</numberToReturn></internalQuery> $END$