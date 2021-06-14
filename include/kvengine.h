//
// Created by zhanghuigui on 2021/6/11.
//

#pragma once

#include <iostream>
#include <string>

#include "rocksdb/db.h"
#include "rocksdb/options.h"

class KVEngine {
 public:
  KVEngine(std::string path) : path_(path) {}

  void Init() {
    opt_.create_if_missing = true;
    if (Open() != "ok") {
    	std::cout << "Open db failed " << std::endl;
    }
  }

  std::string Open() {
    auto s = rocksdb::DB::Open(opt_, path_, &db_);
    if (!s.ok()) {
      return s.ToString();
    }
    return "ok";
  }

  std::string Get(const std::string& key) {
    if (nullptr == db_) {
      return "db_ is nullptr, please init it.";
    }

    std::string tmp_val;
    auto s = db_->Get(rocksdb::ReadOptions(), key, &tmp_val);
    if (!s.ok()) {
      return "not ok";
    }

    return tmp_val;
  }

  std::string Put(const std::string& key, const std::string& val) {
    if (nullptr == db_) {
      return "db_ is nullptr, please init it.";
    }

    auto s = db_->Put(rocksdb::WriteOptions(), key, val);
    if (!s.ok()) {
      std::cout << "Put failed " << s.ToString() << std::endl;
      return s.ToString();
    }

    return "ok";
  }

 private:
  std::string path_;
  rocksdb::DB* db_;
  rocksdb::Options opt_;
};
