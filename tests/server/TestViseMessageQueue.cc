/** @file   ViseMessageQueue.h
 *  @brief  producer consumer message queue
 *
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   18 April 2017
 */

#include <iostream>
#include <string>

#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <boost/lexical_cast.hpp>
#include <unistd.h>

ViseMessageQueue msg_queue;

void Producer() {
  unsigned int count = 1;
  unsigned int rand_wait;
  std::string thread_id = boost::lexical_cast<std::string>(boost::this_thread::get_id());
  for ( unsigned int i=0; i<100; i++) {
    std::ostringstream s;
    rand_wait = 500 + (rand() % 2000) + 1;
    s << "[Thread " << thread_id << ", wait = " << rand_wait << " ms] Message " << count;
    msg_queue.Push( s.str() );
    count += 1;
    usleep(rand_wait);
  }
}

void Consumer() {
  unsigned int count = 0;
  while(1) {
    std::string s;
    msg_queue.Pop( s );
    count += 1;
    std::cout << "\n[" << count << "] Received message : " << s << std::flush;
  }
}

int main(int argc, char** argv) {
  srand (time(NULL));
  boost::thread_group producer_threads, consumer_threads;

  for (int i = 0; i != 100; ++i){
    producer_threads.create_thread(Producer);
  }

  consumer_threads.create_thread(Consumer);

  producer_threads.join_all();
  consumer_threads.join_all();
  std::cout << "\nDone" << std::endl;
}

