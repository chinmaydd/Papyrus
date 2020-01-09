#ifndef PAPYRUS_LOGGER_H
#define PAPYRUS_LOGGER_H

// Code motivated from: https://stackoverflow.com/a/32262143
// TODO: Think if this has to be a namespace?
//       Or does it go under the papyrus namespace
//       What are the implications of that decision?
//       Technically, a namespace might be ideal to separate interface and 
//       implementation. In this case however, I do not see how multiple 
//       implementations of the "Logger" might be substituted.
#include <iostream>

enum typelog {
  DEBUG,
  INFO,
  WARN,
  ERROR
};

struct structlog {
  bool headers = false;
  typelog level = WARN;
};

extern structlog LOGCFG;

class LOG {
public:
  LOG() {};

  LOG(typelog type) {
    msglevel = type;
    if(LOGCFG.headers) {
      operator << ("[" + getLabel(type) + "] ");
    }
  }

  ~LOG() {
    if(opened) {
      std::cout << std::endl;
    }
    opened = false;
  }

  template<class T>
  LOG &operator<<(const T &msg) {
    if(msglevel >= LOGCFG.level) {
      std::cout << msg;
      opened = true;
    }
    return *this;
  }
private:
  bool opened = false;
  typelog msglevel = DEBUG;
  inline std::string getLabel(typelog type) {
    std::string label;
    switch(type) {
      case DEBUG: label = "DEBUG"; break;
      case INFO:  label = "INFO"; break;
      case WARN:  label = "WARN"; break;
      case ERROR: label = "ERROR"; break;
    }
    return label;
  }
};

#endif /* PAPYRUS_LOGGER_H */
