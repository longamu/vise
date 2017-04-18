#include "ViseMessageQueue.h"

void ViseMessageQueue::Push( const std::string &d ) {
  boost::lock_guard<boost::mutex> guard(mtx_);
  messages_.push( d );
  queue_condition_.notify_one();
}

void ViseMessageQueue::BlockingPop( std::string &d ) {
  // block if message queue is empty
  boost::mutex::scoped_lock lock(mtx_);
  while ( messages_.empty() ) {
    queue_condition_.wait(lock);
  }

  d = messages_.front();
  messages_.pop();
}
