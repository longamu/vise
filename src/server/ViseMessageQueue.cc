#include "ViseMessageQueue.h"

void ViseMessageQueue::Push( const std::string &d ) {
  //boost::lock_guard<boost::mutex> guard(mtx_);
  boost::mutex::scoped_lock lock(mtx_);
  messages_.push( d );
  lock.unlock();
  queue_condition_.notify_one();
}

std::string ViseMessageQueue::BlockingPop() {
  boost::mutex::scoped_lock lock(mtx_);
  while ( messages_.empty() ) {
    queue_condition_.wait(lock);
  }
  std::string d = messages_.front();
  messages_.pop();
  return d;
}

unsigned int ViseMessageQueue::GetSize() {
  boost::mutex::scoped_lock lock(mtx_);
  return messages_.size();
}

void ViseMessageQueue::WaitUntilEmpty() {
  boost::mutex::scoped_lock lock(mtx_);
  while ( !messages_.empty() ) {
    queue_condition_.wait(lock);
  }
}
