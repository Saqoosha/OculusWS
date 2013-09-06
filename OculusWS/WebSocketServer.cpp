//
//  WebSocketServer.mm
//  OculusWS
//
//  Created by Saqoosha on 2013/08/22.
//  Copyright (c) 2013 Saqoosha. All rights reserved.
//

#include "WebSocketServer.h"
#include <syslog.h>
#include "json.h"


static const char *reason_names[] = {
  "LWS_CALLBACK_ESTABLISHED",
  "LWS_CALLBACK_CLIENT_CONNECTION_ERROR",
  "LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH",
  "LWS_CALLBACK_CLIENT_ESTABLISHED",
  "LWS_CALLBACK_CLOSED",
  "LWS_CALLBACK_CLOSED_HTTP",
  "LWS_CALLBACK_RECEIVE",
  "LWS_CALLBACK_CLIENT_RECEIVE",
  "LWS_CALLBACK_CLIENT_RECEIVE_PONG",
  "LWS_CALLBACK_CLIENT_WRITEABLE",
  "LWS_CALLBACK_SERVER_WRITEABLE",
  "LWS_CALLBACK_HTTP",
  "LWS_CALLBACK_HTTP_FILE_COMPLETION",
  "LWS_CALLBACK_HTTP_WRITEABLE",
  "LWS_CALLBACK_FILTER_NETWORK_CONNECTION",
  "LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION",
  "LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS",
  "LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS",
  "LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION",
  "LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER",
  "LWS_CALLBACK_CONFIRM_EXTENSION_OKAY",
  "LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED",
  "LWS_CALLBACK_PROTOCOL_INIT",
  "LWS_CALLBACK_PROTOCOL_DESTROY",
  "LWS_CALLBACK_ADD_POLL_FD",
  "LWS_CALLBACK_DEL_POLL_FD",
  "LWS_CALLBACK_SET_MODE_POLL_FD",
  "LWS_CALLBACK_CLEAR_MODE_POLL_FD",
};


#define MAX_JSON_LENGTH 2048
static unsigned char json_string[LWS_SEND_BUFFER_PRE_PADDING + MAX_JSON_LENGTH + LWS_SEND_BUFFER_POST_PADDING];
static size_t json_string_length = 0;

static int callback_lws(struct libwebsocket_context *context,
                        struct libwebsocket *wsi,
                        enum libwebsocket_callback_reasons reason,
                        void *user,
                        void *in,
                        size_t len) {
//  std::cerr << "callback_lws: reason: " << reason << " " << reason_names[reason] << std::endl;
  switch (reason) {
    case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
    case LWS_CALLBACK_FILTER_NETWORK_CONNECTION: {
      return 0;
    }
    case LWS_CALLBACK_HTTP: {
      // following code is borrowed from ofxLibwebsockets ;-)
      
      std::string url((char*)in);
      if (url == "/") url = "/index.html";
      
      // why does this need to be done?
      std::string ext = url.substr(url.find_last_of(".") + 1);
      // watch out for query strings!
      size_t find = url.find("?");
      if (find != std::string::npos) {
        url = url.substr(0, url.find("?"));
      }
      std::string file = "htdocs" + url;
      std::string mimetype = "text/html";
      
      if (ext == "ico")
        mimetype = "image/x-icon";
      if (ext == "manifest")
        mimetype = "text/cache-manifest";
      if (ext == "swf")
        mimetype = "application/x-shockwave-flash";
      if (ext == "js")
        mimetype = "application/javascript";
      if (ext == "css")
        mimetype = "text/css";
      
      if (libwebsockets_serve_http_file(context, wsi, file.c_str(), mimetype.c_str())) {
//        std::cerr << "Failed to send HTTP file " + file + " for " + url << std::endl;
      }
      return 0;
    }
    case LWS_CALLBACK_HTTP_WRITEABLE: {
      libwebsocket_callback_on_writable(context, wsi);
      return 0;
    }
    case LWS_CALLBACK_ESTABLISHED:
    case LWS_CALLBACK_CLOSED: {
      return 0;
    }
    case LWS_CALLBACK_SERVER_WRITEABLE: {
      libwebsocket_write(wsi, &json_string[LWS_SEND_BUFFER_PRE_PADDING], json_string_length, LWS_WRITE_TEXT);
      return 0;
    }
    case LWS_CALLBACK_RECEIVE: {
      Json::Value root;
      Json::Reader reader;
      if (reader.parse((char *)in, (char *)in + len, root)) {
        WebSocketServer *server = (WebSocketServer *)libwebsocket_context_user(context);
        server->received()(root);
      }
      break;
    }
    default: {
      return 0;
    }
  }
  return 1;
}



WebSocketServer::WebSocketServer()
  : force_exit_(false) {
  pthread_mutex_init(&mutex_, NULL);
}


WebSocketServer::~WebSocketServer() {
}


static struct libwebsocket_protocols protocols[] = {
  {NULL, callback_lws, 0},
  {NULL, NULL, 0}
};

void WebSocketServer::start(int port) {
  lws_set_log_level(LOG_DEBUG, lwsl_emit_syslog);

  dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
  group_ = dispatch_group_create();
  dispatch_group_async(group_, queue, ^{
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    info.port = port;
    info.iface = "";

    info.protocols = protocols;
    info.extensions = libwebsocket_get_internal_extensions();
    info.gid = -1;
    info.uid = -1;
    info.options = 0;
    info.user = this;
    
    context_ = libwebsocket_create_context(&info);
    if (context_ == NULL) {
      lwsl_err("libwebsocket init failed\n");
      return;
    }
    std::cout << "libwebsocket create context succeded!" << std::endl;
  
    int n = 0;
    while (n >= 0 && !force_exit_) {
      pthread_mutex_lock(&mutex_);
      if (message_queue_.size()) {
        std::string message = message_queue_.back();
        message_queue_.pop_back();
        json_string_length = message.length();
        strncpy((char *)&json_string[LWS_SEND_BUFFER_PRE_PADDING], message.c_str(), MAX_JSON_LENGTH);
        libwebsocket_callback_on_writable_all_protocol(&protocols[0]);
      }
      pthread_mutex_unlock(&mutex_);
      n = libwebsocket_service(context_, 5);
    }
  });
}


void WebSocketServer::send(std::string message) {
  pthread_mutex_lock(&mutex_);
  message_queue_.push_back(message);
  pthread_mutex_unlock(&mutex_);
}
