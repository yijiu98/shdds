#pragma once
#include <mutex>

#define DISALLOW_COPY_AND_ASSIGN(classname) \
  classname(const classname &) = delete; \
  classname & operator=(const classname &) = delete;

#define DECLARE_SINGLETON(classname) \
public: \
  static classname * Instance() { \
    static classname * instance = nullptr; \
    if(!instance) \
      instance = new (std::nothrow) classname(); \
    return instance; \
  } \
 \
private: \
  classname(); \
  DISALLOW_COPY_AND_ASSIGN(classname)


