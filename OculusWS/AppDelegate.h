//
//  AppDelegate.h
//  OculusWS
//
//  Created by Saqoosha on 2013/08/22.
//  Copyright (c) 2013 Saqoosha. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include <boost/bind.hpp>
#include "SensorMain.h"
#include "WebSocketServer.h"


@interface WebServer : NSObject
{
  NSTask *task_;
}
@end


class Router {
 public:
  Router(WebSocketServer *server, SensorMain *sensor) {
    server_ = server;
    server_->received().connect(boost::bind(&Router::onReceivedFromWebSocket, this, _1));
    sensor_ = sensor;
  }
  
 private:
  void onReceivedFromWebSocket(Json::Value &value) {
    if (value["address"].asString() == "/sensor/reset") {
      sensor_->reset();
    }
  }
  
  WebSocketServer *server_;
  SensorMain *sensor_;
};


@interface AppDelegate : NSObject <NSApplicationDelegate>
{
  SensorMain *sensor_;
  WebSocketServer *socketServer_;
  Router *router_;
}

- (IBAction)startServer:(id)sender;
- (IBAction)resetSensor:(id)sender;

@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet NSButton *startButton;
@property (assign) IBOutlet NSTextField *status;
@property (assign) BOOL running;

@end
