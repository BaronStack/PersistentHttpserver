#include "../include/http_server.h"
#include "../include/http_url_factory.h"

KVEngine* HttpServer::kv_engine_ = nullptr;

void HttpServer::Init(const std::string &port) {
	std::string tmp_port = port;
	if (!CheckPort(tmp_port)) {
		printf("There is no port to use.\n");
		exit(-1);
	}

	m_port_ = tmp_port;
	s_server_option.enable_directory_listing = "yes";
	s_server_option.document_root = s_web_dir.c_str();

	kv_engine_ = new KVEngine("./db"); // current db's directory
	kv_engine_->Init();
}

bool HttpServer::Start() {
	mg_mgr_init(&m_mgr_, nullptr);
	mg_connection *connection = mg_bind(&m_mgr_, m_port_.c_str(), HttpServer::OnHttpEvent);
	if (connection == nullptr)
		return false;
	// for both http and websocket
	mg_set_protocol_http_websocket(connection);

	printf("starting http server at port: %s\n", m_port_.c_str());
	// loop
	while (true)
		mg_mgr_poll(&m_mgr_, 500); // ms
}

void HttpServer::OnHttpEvent(mg_connection *connection, int event_type, void *event_data) {
	if (event_type == MG_EV_HTTP_REQUEST) {
		http_message *http_req = (http_message *)event_data;
		HandleHttpEvent(connection, http_req);
	}
}

// ---- simple http ---- //
static bool route_check(http_message *http_msg, const char *route_prefix) {
	if (mg_vcmp(&http_msg->uri, route_prefix) == 0)
		return true;
	else
		return false;
}

void HttpServer::SendHttpRsp(mg_connection *connection, std::string rsp) {
	mg_printf(connection, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
	mg_printf_http_chunk(connection, "{ \"result\": %s }", rsp.c_str());
	mg_send_http_chunk(connection, "", 0);
}

void HttpServer::HandleHttpEvent(mg_connection *connection, http_message *http_req) {
	std::string req_str = std::string(http_req->message.p, http_req->message.len);
	std::string url = std::string(http_req->uri.p, http_req->uri.len);
	printf("got request: %s\n", req_str.c_str());
	InitializeAllOp(connection, http_req);

	// Register the operation for the url
	auto *judge = new JudgeOperation(connection, http_req);
	auto res = judge->Judge(url);
	if (res != "ok") {
		SendHttpRsp(connection, res);
	}
}

bool HttpServer::Close() {
	mg_mgr_free(&m_mgr_);
	return true;
}
