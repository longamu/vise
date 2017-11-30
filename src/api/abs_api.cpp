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

#include "abs_api.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <vector>
#include <stdexcept>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "ViseMessageQueue.h"
#include "timing.h"

void
absAPI::session( socket_ptr sock ){

    // Read the client's request.
    std::string request = "";
    while (1) {
        char buffer[1024];
        //boost::asio::error error;
        boost::system::error_code error;
        size_t len = sock->read_some(boost::asio::buffer(buffer), error);
        if (error == boost::asio::error::eof) { // Connection closed by peer.
            // Nothing to do, return.
            return;
        }
        else if (error) {
            std::cerr << error.message() << "\n";
            throw error; // Some error?
        }
        request+= std::string(buffer, len);
        std::string requestTrim= request;
        boost::algorithm::trim_right(requestTrim);

        if (std::string(requestTrim.begin() + requestTrim.size() - 6, requestTrim.end()) == " $END$"){
            request= requestTrim;
            break;
        }
    }

    request= request.substr(0, request.length()-6); // remove end

    double t0= timing::tic();

    // parse the request
    std::stringstream ss( request );

    boost::property_tree::ptree pt;
    read_xml( ss, pt );

    std::string reply;

    if ( pt.count("dsetGetNumDocs") ){

        reply= ( boost::format("%d") % dataset_->getNumDoc() ).str();

    } else if ( pt.count("dsetGetFn") ){

        uint32_t docID= pt.get<uint32_t>("dsetGetFn.docID");
        reply= ( boost::format("%d") % dataset_->getFn(docID) ).str();

    } else if ( pt.count("dsetGetDocID") ){

        std::string fn= pt.get<std::string>("dsetGetDocID.fn");
        reply= ( boost::format("%d") % dataset_->getDocIDFromAbsFn(fn) ).str();;

    } else if ( pt.count("dsetGetWidthHeight") ){

        uint32_t docID= pt.get<uint32_t>("dsetGetWidthHeight.docID");
        std::pair<uint32_t, uint32_t> wh= dataset_->getWidthHeight(docID);
        reply= ( boost::format("%d %d") % wh.first % wh.second ).str();

    } else if ( pt.count("containsFn") ){

        std::string fn= pt.get<std::string>("containsFn.fn");
        reply= ( boost::format("%d") % dataset_->containsFn(fn) ).str();

    } else {

//         std::cout<< timing::getTimeString() <<" Request= "<<request<<"\n";
//        std::cout<< timing::getTimeString() <<" Request= "<< request.substr(0,300) << ( request.length()>300 ? " (...) \n" : "\n" ) ;

        reply= getReply(pt, request);

//         std::cout<<"Response= "<<reply<<"\n";
//        std::cout<<"Response= "<< reply.substr(0,300) << ( reply.length()>300 ? " (...) \n" : "\n" ) ;
        std::cout<<timing::getTimeString()<<" Request - DONE ("<< timing::toc(t0) <<" ms)\n";
    }

    boost::asio::write(*sock, boost::asio::buffer(reply));
}

void exec_cmd(const std::string cmd, std::string& output) {
  std::array<char, 128> buffer;
  std::ostringstream cmd_output;

  std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
  if (!pipe) throw std::runtime_error("popen() failed!");

  while (!feof(pipe.get())) {
    if (fgets(buffer.data(), 80, pipe.get()) != nullptr) {
      cmd_output << buffer.data();
    }
  }
  output = cmd_output.str();
}

void InitReljaRetrivalFrontend(std::string dsetname, std::string configFn, std::string vise_src_code_dir) {
  // @todo: avoid relative path and discover the file "compute_clusters.py" automatically
  std::string exec_name = vise_src_code_dir + "/src/ui/web/webserver2.py";

  std::cout << "\nLoading frontend using : " << exec_name << std::flush;
  std::ostringstream s;
  s << "python " << exec_name << " 9973";
  s << " " << dsetname;
  s << " 65521";
  s << " " << configFn;
  s << " true";

  std::array<char, 128> buffer;
  std::ostringstream cmd_output;

  std::cout << "\nexecuting command: " << s.str() << std::endl << std::flush;
  std::shared_ptr<FILE> pipe(popen(s.str().c_str(), "r"), pclose);
  if (!pipe) throw std::runtime_error("popen() failed!");

/*
  // @todo: simplify this in future with a better designed code
  // check where the frontend is running
  boost::this_thread::sleep_for(boost::chrono::milliseconds(1500)); // wait until frontend starts
  std::string cmd1 = "curl -I -X GET http://localhost:9973";
  std::string cmd2 = "curl -I -X GET http://0.0.0.0:9973";
  std::string cmd1_output, cmd2_output;
  exec_cmd(cmd1, cmd1_output);
  exec_cmd(cmd2, cmd2_output);
  std::cout << "\ncmd1_output = " << cmd1_output << std::flush;
  std::cout << "\ncmd2_output = " << cmd2_output << std::flush;
  std::string success_str("HTTP/1.1 200 OK");
  std::size_t found1 = cmd1_output.find(success_str);
  std::size_t found2 = cmd2_output.find(success_str);

  if(found1 != std::string::npos) {
    ViseMessageQueue::Instance()->Push("Query command _redirect http://localhost:9973 500");
  } else {
    if(found2 != std::string::npos) {
      ViseMessageQueue::Instance()->Push("Query command _redirect http://0.0.0.0:9973 500");
    } else {
      ViseMessageQueue::Instance()->Push("Query command _redirect http://INSERT_YOUR_IP_HERE:9973 500");
    }
  }
*/
  ViseMessageQueue::Instance()->Push("Query command _redirect http://0.0.0.0:9973 1000");
  while (!feof(pipe.get())) {
    if (fgets(buffer.data(), 80, pipe.get()) != nullptr) {
      cmd_output << buffer.data();
      std::cout << buffer.data() << std::flush;
    }
  }
  std::cout << " [done]" << std::flush;
}

void
absAPI::server(boost::asio::io_service& io_service, unsigned int port, std::string dsetname, std::string configFn, std::string vise_src_code_dir) {

    std::cout << "\nabsAPI::server() : Waiting for requests on port " << port << std::flush;
try_again:
    try {
        tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));
        a.set_option(tcp::acceptor::reuse_address(true));


        boost::thread t( boost::bind( &InitReljaRetrivalFrontend, dsetname, configFn, vise_src_code_dir ) );

        while (1) {
            socket_ptr sock(new tcp::socket(io_service));
            a.accept(*sock);
            boost::thread t(boost::bind(&absAPI::session, this, sock));
        }
    }
    catch (std::exception& e) {
        std::cerr<<"\nabsAPI::server() : "<< e.what() << std::flush;
        sleep(1);
        goto try_again;
    }

}
