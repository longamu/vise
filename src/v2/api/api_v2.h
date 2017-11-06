#ifndef _API_V2_H_
#define _API_V2_H_


#include "spatial_api.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <vector>
#include <stdexcept>
#include <string>

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/lambda/construct.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/thread.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <fastann.hpp>

#include "ViseMessageQueue.h"
#include "clst_centres.h"
#include "dataset_v2.h"
#include "feat_getter.h"
#include "feat_standard.h"
#include "hamming.h"
#include "hamming_embedder.h"
#include "index_entry.pb.h"
#include "macros.h"
#include "mq_filter_outliers.h"
#include "par_queue.h"
#include "proto_db.h"
#include "proto_db_file.h"
#include "proto_index.h"
#include "python_cfg_to_ini.h"
#include "slow_construction.h"
#include "soft_assigner.h"
#include "spatial_verif_v2.h"
#include "tfidf_data.pb.h"
#include "tfidf_v2.h"
#include "util.h"

void api_v2(std::vector< std::string > argv);

#endif
