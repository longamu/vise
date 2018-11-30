/*
==== Author:

Relja Arandjelovic (relja@robots.ox.ac.uk)
Visual Geometry Group,
Department of Engineering Science
University of Oxford

==== Copyright:

The library belongs to Relja Arandjelovic and the University of Oxford.
No usage or redistribution is allowed without explicit permission.
*/

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "dataset_v2.h"
#include "hamming.h"
#include "hamming_embedder.h"
#include "mpi_queue.h"
#include "image_graph.h"
#include "proto_db.h"
#include "proto_db_file.h"
#include "proto_index.h"
#include "spatial_verif_v2.h"
#include "tfidf_v2.h"
#include "util.h"


int main(int argc, char* argv[]){
    
    // MPI initialization
    MPI_INIT_ENV
    MPI_GLOBAL_ALL
    
    // file name
    std::string imageGraphFn= util::expandUser("/data/seebibyte_tap/fleuron/vise_data/data/fleuron/2/imgraph_nbd999_th3.v2bin");
    
    // create the image graph
    
    // file names
    
    std::string iidxFn= util::expandUser("/data/seebibyte_tap/fleuron/vise_data/data/fleuron/2/iidx.v2bin");
    std::string fidxFn= util::expandUser("/data/seebibyte_tap/fleuron/vise_data/data/fleuron/2/fidx.v2bin");
    std::string wghtFn= util::expandUser("/data/seebibyte_tap/fleuron/vise_data/data/fleuron/2/wght.v2bin");
    std::string hammFn= util::expandUser("/data/seebibyte_tap/fleuron/vise_data/data/fleuron/2/hamm.v2bin");
    
    if ( rank == 0 ) {
      std::cout << "\nBuilding image graph (numProc=" << numProc << ") ..." << std::flush;
      std::cout << "\n  iidx = " << iidxFn;
      std::cout << "\n  fidx = " << fidxFn;
      std::cout << "\n  wght = " << wghtFn;
      std::cout << "\n  hamm = " << hammFn << std::endl;
    }

    // load fidx
    
    protoDbFile dbFidx_file(fidxFn);
    protoDbInRam dbFidx(dbFidx_file, rank==0);
    protoIndex fidx(dbFidx, false);
    
    // load iidx
    
    protoDbFile dbIidx_file(iidxFn);
    protoDbInRam dbIidx(dbIidx_file, rank==0);
    protoIndex iidx(dbIidx, false);
    
    // create retriever
    
    tfidfV2 tfidfObj(&iidx, &fidx, wghtFn);
    hammingEmbedderFactory embFactory(hammFn, 64);
    hamming hammingObj(tfidfObj, &iidx, embFactory, &fidx);
    spatialVerifV2 spatVerifHamm(hammingObj, &iidx, &fidx, true);
    
    imageGraph imGraph;
    // compute image graph in parallel
    imGraph.computeParallel(imageGraphFn, fidx.numIDs(), spatVerifHamm, 999, 3 );
    //imGraph.computeSingle(imageGraphFn, fidx.numIDs(), spatVerifHamm, 9999, 3 );

    // required clean up for protocol buffers
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
