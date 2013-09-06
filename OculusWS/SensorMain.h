//
//  SensorMain.h
//  OculusWS
//
//  Created by Saqoosha on 2013/08/22.
//  Copyright (c) 2013 Saqoosha. All rights reserved.
//

#ifndef __OculusWS__SensorMain__
#define __OculusWS__SensorMain__

#include <iostream>
#include "OVR.h"
#include "WebSocketServer.h"
#include "json.h"

using namespace OVR;

class SensorMain : public MessageHandler {
 public:
  SensorMain(WebSocketServer *server);
  ~SensorMain();
  void reset();
  bool running() { return pSensor; }
  
 protected:
  void OnMessage(const Message &msg);

  Ptr<DeviceManager> pManager;
  Ptr<SensorDevice> pSensor;
  SensorFusion FusionResult;

  WebSocketServer *server_;
  Json::FastWriter writer_;
};

#endif /* defined(__OculusWS__SensorMain__) */
