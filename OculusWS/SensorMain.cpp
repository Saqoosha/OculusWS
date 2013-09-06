//
//  SensorMain.cpp
//  OculusWS
//
//  Created by Saqoosha on 2013/08/22.
//  Copyright (c) 2013 Saqoosha. All rights reserved.
//

#include "SensorMain.h"

SensorMain::SensorMain(WebSocketServer *server) {
  server_ = server;
  
  pManager = *DeviceManager::Create();
  pSensor = pManager->EnumerateDevices<SensorDevice>().CreateDevice();
  if (pSensor) {
    pSensor->SetReportRate(60 * 2);
    std::cerr << "Report rate: " << pSensor->GetReportRate() << std::endl;
    FusionResult.AttachToSensor(pSensor);
    FusionResult.SetDelegateMessageHandler(this);
//    FusionResult.SetPrediction(1.0 / 60.0);
  } else {
    std::cerr << "No sensor?" << std::endl;
  }
}


SensorMain::~SensorMain() {
  RemoveHandlerFromDevices();
  pSensor.Clear();
  pManager.Clear();
}


void SensorMain::reset() {
  FusionResult.Reset();
}


void SensorMain::OnMessage(const OVR::Message &msg) {
  if (msg.Type == Message_BodyFrame) {
    Quatf q = FusionResult.GetPredictedOrientation();
    {
      Json::Value root;
      root["address"] = "/rift/orientation";
      Json::Value args;
      args.append(q.x);
      args.append(q.y);
      args.append(q.z);
      args.append(q.w);
      root["args"] = args;
      server_->send(writer_.write(root));
    }
    {
      float x, y, z;
      q.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&y, &x, &z);
      Json::Value root;
      root["address"] = "/rift/eularangles";
      Json::Value args;
      args.append(x);
      args.append(y);
      args.append(z);
      root["args"] = args;
      server_->send(writer_.write(root));
    }
    {
      Vector3f acc = FusionResult.GetAcceleration();
      Json::Value root;
      root["address"] = "/rift/acceleration";
      Json::Value args;
      args.append(acc.x);
      args.append(acc.y);
      args.append(acc.z);
      root["args"] = args;
      server_->send(writer_.write(root));
    }
  }
}
