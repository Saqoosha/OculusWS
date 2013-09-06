//
//  WebSocketServer.h
//  OculusWS
//
//  Created by Saqoosha on 2013/08/22.
//  Copyright (c) 2013 Saqoosha. All rights reserved.
//

#ifndef __OculusWS__WebSocketServer__
#define __OculusWS__WebSocketServer__

#include <iostream>
#include <deque>
#include <dispatch/dispatch.h>
#include <pthread.h>
#include <boost/signals2/signal.hpp>
#include "libwebsockets.h"
#include "json.h"


class WebSocketServer {
 public:
  WebSocketServer();
  ~WebSocketServer();
  
  void start(int port);
  void send(std::string message);

  boost::signals2::signal<void (Json::Value&)> &received() { return received_; }

 protected:
  struct libwebsocket_context *context_;
  dispatch_group_t group_;
  bool force_exit_;
  std::deque<std::string> message_queue_;
  pthread_mutex_t mutex_;
  boost::signals2::signal<void (Json::Value&)> received_;
};

#endif /* defined(__OculusWS__WebSocketServer__) */
