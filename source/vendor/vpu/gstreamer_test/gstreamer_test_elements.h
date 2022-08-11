#pragma once
#include <string>

#include "json/json.h"

class GstElements {
 public:
  static GstElements *shared();

  bool getVideoEncoder(const char *mime, std::string &element,
                       std::string &caps);

  std::string videoParser(const char *mime);
  bool getVideoDecoder(const char *mime, std::string &element,
                       std::string &caps);
  std::string videoConverter(const char *mime);
  std::string videoRenderer(const char *mime);
  std::string videoTests();

 private:
  GstElements(const Json::Value &value);
  ~GstElements();

 private:
  Json::Value root;
};
