#ifndef SCRIPT_H_
#define SCRIPT_H_

#include "lua.hpp"

#include "utils.h"

#include <string>

class Script {
 public:
  Script();
  ~Script();

  bool loadScript(const std::string &scriptfile);

  std::string getGlobalString(const std::string &name);
  void setGlobalString(const std::string &name, const std::string &value);

  double getGlobalNumber(const std::string &name);
  void setGlobalNumber(const std::string &name, const double &value);

  bool getGlobalBoolean(const std::string &name);
  void setGlobalBoolean(const std::string &name, const bool &value);

  void runFunction(const std::string &name);

 private:
  lua_State *lua_state_;
  DISALLOW_COPY_AND_ASSIGN(Script);
};

#endif // SCRIPT_H_
