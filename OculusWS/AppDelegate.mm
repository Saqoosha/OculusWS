//
//  AppDelegate.m
//  OculusWS
//
//  Created by Saqoosha on 2013/08/22.
//  Copyright (c) 2013 Saqoosha. All rights reserved.
//

#import "AppDelegate.h"
#include <iostream>



@implementation AppDelegate

@synthesize running;

- (void)awakeFromNib
{
//  NSLog([[NSBundle mainBundle] bundlePath]);
  running = NO;
  [[NSUserDefaults standardUserDefaults] registerDefaults:@{
    @"webServer": @"3000",
    @"webSocketServer": @"7681",
    @"oscReceiver": @"8111",
    @"oscSender": @"8000"
   }];
}

- (void)dealloc
{
  [super dealloc];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  [self startServer:nil];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication
{
  return YES;
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
  if (running) [self startServer:nil];
}

- (IBAction)startServer:(id)sender
{
  [self willChangeValueForKey:@"running"];
  if (running)
  {
    delete sensor_;
    sensor_ = 0;
    delete socketServer_;
    socketServer_ = 0;
    delete router_;
    router_ = 0;
    _status.stringValue = @"Stopped";
    running = NO;
    _startButton.title = @"Start Server";
  }
  else
  {
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    socketServer_ = new WebSocketServer();
    socketServer_->start([[defaults objectForKey:@"webSocketServer"] intValue]);
    sensor_ = new SensorMain(socketServer_);
    if (sensor_->running())
    {
      _status.stringValue = @"Running";
    }
    else
    {
      _status.stringValue = @"Oculus Rift isn't connected...?";
    }
    router_ = new Router(socketServer_, sensor_);
    running = YES;
    _startButton.title = @"Stop Server";
  }
  [self didChangeValueForKey:@"running"];
}

- (IBAction)resetSensor:(id)sender
{
  if (sensor_) sensor_->reset();
}

@end
