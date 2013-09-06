//
//  main.m
//  OculusWS
//
//  Created by Saqoosha on 2013/08/22.
//  Copyright (c) 2013 Saqoosha. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include "OVR.h"

int main(int argc, char *argv[])
{
  OVR::System::Init();
  NSApplicationMain(argc, (const char **)argv);
  OVR::System::Destroy();
  return 0;
}
