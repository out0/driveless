#ifndef _LOGGER_H
#define _LOGGER_H

#include <string>

using namespace std;

class Logger {
public:
    virtual void info(const char *format, ...) {}
    virtual void warning(const char *format, ...) {}
    virtual void error(const char *format, ...) {}
    virtual void critical(const char *format, ...) {};
};

#endif