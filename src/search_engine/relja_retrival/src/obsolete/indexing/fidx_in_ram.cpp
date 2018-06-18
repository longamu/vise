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

#include "fidx_in_ram.h"

#include "thread_queue.h"

#include "timing.h"
#include "macros.h"


class loaderWorker : public queueWorker<bool> {
    public:
        
        loaderWorker(forwardIndex const &fidx, uint32_t *num, uint32_t **wordIDs, float **x, float **y, float **a, float **b, float **c) :
            fidx_(&fidx), num_(num), wordIDs_(wordIDs), x_(x), y_(y), a_(a), b_(b), c_(c) {}
        
        void operator() (uint32_t docID, bool &result) const {
            std::vector<quantDesc> words;
            std::vector<ellipse> regions;
            fidx_->getWordsRegs( docID, words, regions );
            
            uint32_t N= words.size();
            num_[docID]= N;
            wordIDs_[docID]= new uint32_t[N];
            x_[docID]= new float[N];
            y_[docID]= new float[N];
            a_[docID]= new float[N];
            b_[docID]= new float[N];
            c_[docID]= new float[N];
            for (uint32_t i= 0; i<N; ++i){
                ASSERT(words[i].rep.size()==1);
                wordIDs_[docID][i]= words[i].rep[0].first;
                ellipse const &E= regions[i];
                x_[docID][i]= E.x;
                y_[docID][i]= E.y;
                a_[docID][i]= E.a;
                b_[docID][i]= E.b;
                c_[docID][i]= E.c;
            }
        }
    
    private:
        forwardIndex const *fidx_;
        uint32_t *num_, **wordIDs_;
        float **x_, **y_, **a_, **b_, **c_;
};

class loaderManager : public queueManager<bool> {
    public:
        loaderManager(uint32_t numDocs) : time_(timing::tic()), numDone_(0), numDocs_(numDocs) {
            numDocs_printStep_= std::max(static_cast<uint32_t>(1),numDocs_/20);
        }
        void operator() ( uint32_t jobID, bool &result ) {
            ++numDone_;
            if (numDone_ % numDocs_printStep_ == 0)
                std::cout<<"fidxInRam::fidxInRam: loading docID= "<<numDone_<<" / "<<numDocs_<<" "<<timing::toc(time_)<<" ms\n";
        }
    private:
        double time_;
        uint32_t numDone_, numDocs_, numDocs_printStep_;
};



fidxInRam::fidxInRam( forwardIndex const &fidx ) : numDocs_(fidx.numDocs()) {
    
    num= new uint32_t[numDocs_];
    wordIDs= new uint32_t*[numDocs_];
    x= new float*[numDocs_];
    y= new float*[numDocs_];
    a= new float*[numDocs_];
    b= new float*[numDocs_];
    c= new float*[numDocs_];
    
    std::cout<<"fidxInRam::fidxInRam: loading forward index\n";
    double time= timing::tic();
    {
        loaderWorker worker(fidx, num, wordIDs, x, y, a, b, c);
        loaderManager manager(numDocs_);
        threadQueue<bool>::start(numDocs_, worker, manager, 4);
    }
    
    std::cout<<"fidxInRam::fidxInRam: loading forward index - DONE ("<<timing::toc(time)<<" ms)\n";
    
}
