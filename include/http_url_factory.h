//
// Created by zhanghuigui on 2021/6/11.
//

#pragma once

#include <map>
#include <memory>
#include <string>
#include <iostream>
#include "mongoose.h"

#include "http_server.h"
// Below is a factory pattern template with single pattern

// Register the operation
// The 'OperationTyple_t' is the abstract class
template <class OperationType_t>
class OperationRegister {
public:
  virtual OperationType_t* CreateOperation(
  const std::string& op_name, mg_connection* conn, http_message* hm) = 0;

protected:
  OperationRegister() = default;
  virtual ~OperationRegister() = default;
};

// Factory class template
template <class OperationType_t>
class OperationFactory {
public:
  // Single pattern of the factory
  static OperationFactory<OperationType_t>& Instance() {
	  static OperationFactory<OperationType_t> instance;
	  return instance;
  }

  void RegisterOperation(const std::string& op_name,
						 mg_connection* conn, http_message* hm,
						 OperationRegister<OperationType_t>* reg) {
	  operationRegister[op_name] = reg;
  }

  OperationType_t* GetOperation(
  const std::string& op_name,
  mg_connection* conn, http_message* hm) {
	  if (operationRegister.find(op_name) != operationRegister.end()) {
		  return operationRegister[op_name]->CreateOperation(op_name, conn, hm);
	  }

	  return nullptr;
  }

private:
  // We don't allow to constructor, copy constructor and align constructor
  OperationFactory() = default;
  ~OperationFactory() = default;

  OperationFactory(const OperationFactory&) = delete;
  const OperationFactory& operator= (const OperationFactory&) = delete;

  std::map<std::string, OperationRegister<OperationType_t>* > operationRegister;
};

// An template class to create the detail Operation
template <class OperationType_t, class OperationImpl_t>
class OperationImplRegister : public OperationRegister<OperationType_t> {
public:
  explicit OperationImplRegister(
  const std::string& op_name, mg_connection* conn, http_message* hm) {
	  OperationFactory<OperationType_t>::Instance().RegisterOperation(op_name, conn, hm, this);
  }

  OperationType_t* CreateOperation(
  const std::string& op_name, mg_connection* conn, http_message* hm) {
	  return new OperationImpl_t(op_name, conn, hm);
  }
};

class Url {
public:
  virtual std::string Name() = 0;
  virtual void DoOp() = 0;
  virtual ~Url(){}
};

class SetValueUrl : public Url {
public:
  SetValueUrl(std::string name, mg_connection* conn, http_message* http):
  	name_(name), conn_(conn), hm_(http) {}

  std::string Name() override { return name_; }
  void DoOp() override {
	std::string res;
	char value[100];
	std::cout << "name :" << name_ << std::endl;
	mg_get_http_var(&hm_->body, "value", value, sizeof(value));
	if (HttpServer::kv_engine_) {
		  std::cout << "set value :" << value << std::endl;
		  res = HttpServer::kv_engine_->Put("setvalue", value);
		  std::cout << "put res : " << res << std::endl;
	}
	HttpServer::SendHttpRsp(conn_, res);
  }

private:
  std::string name_;
  mg_connection* conn_;
  http_message* hm_;
};

class GetValue : public Url {
public:
  GetValue(std::string name, mg_connection* conn, http_message* hm) :
  	name_(name), conn_(conn), hm_(hm) {}

  std::string Name() override { return name_; }
  void DoOp() override {
	std::string res;

	if (HttpServer::kv_engine_) {
		  res = HttpServer::kv_engine_->Get("setvalue");
	}
	HttpServer::SendHttpRsp(conn_, res);
  }

private:
  std::string name_;
  mg_connection* conn_;
  http_message* hm_;
};

class RouteUrl : public Url {
public:
  RouteUrl(std::string name, mg_connection* conn, http_message* hm) :
  	name_(name), conn_(conn), hm_(hm) {}

  std::string Name() override  { return name_; }
  void DoOp() override {
  	std::cout << "input str: " << name_ << std::endl;
	mg_serve_http(conn_, hm_, HttpServer::s_server_option);
  }

private:
  std::string name_;
  mg_connection* conn_;
  http_message* hm_;
};

class JudgeOperation {
public:
  JudgeOperation(mg_connection* conn, http_message* hm) :
  conn_(conn), hm_(hm) {}

  // We just need input a string ,then we could call the different profiling
  // factory's func.
  std::string Judge(const std::string& op_name) {
	  OperationFactory<Url>& fac =
	  OperationFactory<Url>::Instance();

	  std::shared_ptr<Url> op(fac.GetOperation(op_name, conn_, hm_));
	  if (op) {
		  op->DoOp();
		  return "ok";
	  }

	  return "Invalid input with Profiling options!";
  }

private:
  mg_connection* conn_;
  http_message* hm_;
};

// Register all the http request's input string and their Class pointer.
void InitializeAllOp(mg_connection* conn, http_message* hm) {
	static bool initialize = false;
	if (!initialize) {
		static OperationImplRegister<Url, GetValue>
		getValue("/test/getvalue", conn, hm);
		static OperationImplRegister<Url, SetValueUrl>
		setValue("/test/setvalue", conn, hm);
		static OperationImplRegister<Url, RouteUrl>
		routeUrl("/", conn, hm);
		initialize = true;
	}
}
