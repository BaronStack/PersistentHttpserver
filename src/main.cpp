#include <iostream>
#include <memory>
#include "../include/http_server.h"

mg_serve_http_opts HttpServer::s_server_option;
std::string HttpServer::s_web_dir = "./web";

int main(int argc, char *argv[]) 
{
	std::string port = "7999";
	auto http_server = std::make_shared<HttpServer>();
	http_server->Init(port);
	http_server->Start();
	return 0;
}