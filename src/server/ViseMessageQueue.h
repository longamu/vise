/** @file   ViseMessageQueue.h
 *  @brief  Multiple-Producer Multiple-Consumer message queue
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
  void Push( const std::string &d );
  void BlockingPop( std::string &d );

private:
  std::queue< std::string > messages_;
  boost::mutex mtx_;
  boost::condition_variable queue_condition_;
};
#endif /* _VISE_MESSAGE_QUEUE_H */
