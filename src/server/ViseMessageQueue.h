/** @file   ViseMessageQueue.h
 *  @brief  Multiple-Producer Multiple-Consumer message queue based on C++ Singleton design pattern
 *
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   18 April 2017
 */

#ifndef _VISE_MESSAGE_QUEUE_H
#define _VISE_MESSAGE_QUEUE_H

#include <queue>
#include <string>

#include <boost/thread.hpp>

class ViseMessageQueue {
 public:
  static ViseMessageQueue *Instance();

  void Push( const std::string &d );
  std::string BlockingPop();

  // to avoid the cases where the HTTP client is swamped by a large message queue
  void WaitUntilEmpty();
  unsigned int GetSize();

 private:
  // C++ Singleton design pattern
  ViseMessageQueue() { };                                    // to prevent creation of new objects
  ViseMessageQueue( ViseMessageQueue const & ) { };          // prevent copy constructor
  ViseMessageQueue *operator=(ViseMessageQueue const& ) {    // prevent assignment operator
    return 0;
  };
  static ViseMessageQueue* vise_global_message_queue_;

  std::queue< std::string > messages_;
  boost::mutex mtx_;
  boost::condition_variable queue_condition_;
};
#endif /* _VISE_MESSAGE_QUEUE_H */
