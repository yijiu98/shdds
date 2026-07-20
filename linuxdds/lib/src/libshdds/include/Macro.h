#pragma once

#define DISALLOW_COPY_AND_ASSIGN(classname) \
  classname(const classname &) = delete; \
  classname & operator=(const classname &) = delete;

// C++11 magic static: 线程安全且随程序退出自动析构
#define DECLARE_SINGLETON(classname) \
public: \
  static classname * Instance() { \
    static classname instance; \
    return &instance; \
  } \
private: \
  classname(); \
  DISALLOW_COPY_AND_ASSIGN(classname)
