#pragma once

#include <string.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "kvengine.h"
#include "mongoose.h"

typedef void OnRspCallback(mg_connection *c, std::string);

// Check there is other port can be used?
static bool CheckPort(std::string& port) {
	if (port.empty()) {
		return false;
	}

	int64_t fd;
	int64_t new_port;
	struct sockaddr_in addr;
	int64_t checktimes = 0;


	// Try to bind the port and check the port is open
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		return false;
	}

	addr.sin_family = AF_INET;
	new_port = std::stoi(port);
	addr.sin_port = htons(new_port);
	inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr);

	while (bind(fd, (struct sockaddr *)(&addr), sizeof(sockaddr_in)) < 0 &&
		   checktimes < 65536) {
		new_port += 1; // we will find the port for 65536 times
		addr.sin_port = htons(new_port);
		inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr);

		checktimes++;
	}

	// Check port failed.
	if (checktimes >= 65536) {
		return false;
	}
	port = std::to_string(new_port);

	// We don't use the fd, it's better to close the fd.
	close(fd);
	return true;
}

class HttpServer {
 public:
  HttpServer() {}
  ~HttpServer() {
  	Close();
  	if (kv_engine_) {
  		delete kv_engine_;
  		kv_engine_	= nullptr;
  	}
  }
  void Init(const std::string &port);
  bool Start();
  bool Close();
  static void SendHttpRsp(mg_connection *connection, std::string rsp);

  static std::string s_web_dir;
  static mg_serve_http_opts s_server_option;
  static KVEngine *kv_engine_;

private:
  static void OnHttpEvent(mg_connection *connection, int event_type,
                          void *event_data);
  static void HandleHttpEvent(mg_connection *connection,
                              http_message *http_req);

  std::string m_port_;
  mg_mgr m_mgr_;
};
