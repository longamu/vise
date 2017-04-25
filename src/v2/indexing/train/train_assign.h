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

#ifndef _TRAIN_ASSIGN_H_
#define _TRAIN_ASSIGN_H_

#include <string>

#include "ViseMessageQueue.h"

// defined in src/vise.cc
// a global message queue to send communications to client HTTP browser
extern ViseMessageQueue vise_message_queue_;

namespace buildIndex {
    
    void
        computeTrainAssigns(std::string const clstFn,
                            bool const RootSIFT,
                            std::string const trainDescsFn,
                            std::string const trainAssignsFn);
}

#endif
