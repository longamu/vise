#include "ViseMessageQueue.h"

void ViseMessageQueue::Push( const std::string &d ) {
  boost::lock_guard<boost::mutex> guard(mtx_);
  messages_.push( d );
  queue_condition_.notify_one();
}

std::string ViseMessageQueue::BlockingPop() {
  boost::mutex::scoped_lock lock(mtx_);
  while ( messages_.empty() ) {
    queue_condition_.wait(lock);
  }
  std::string d = messages_.front();
  messages_.pop();
  queue_condition_.notify_one();
  return d;
}

void ViseMessageQueue::WaitUntilEmpty() {
  boost::mutex::scoped_lock lock(mtx_);
  while ( !messages_.empty() ) {
    queue_condition_.wait(lock);
  }
}
