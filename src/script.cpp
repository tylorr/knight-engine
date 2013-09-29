// #include "script.h"

// using std::string;

// Script::Script() {
//   lua_state_ = luaL_newstate();
//   luaL_openlibs(lua_state_);
// }

// Script::~Script() {
//   lua_close(lua_state_);
// }

// bool Script::loadScript(const string &scriptfile) {
//   int result;
//   try {
//     result = luaL_dofile(lua_state_, scriptfile.c_str());
//   } catch(...) {
//     return false;
//   }

//   return result == 0 ? true : false;
// }

// string Script::getGlobalString(const string &name) {
//   string value = "";

//   try {
//     lua_getglobal(lua_state_, name.c_str());
//     value = lua_tostring(lua_state_, -1);
//     lua_pop(lua_state_, 1);
//   } catch(...) { }

//   return value;
// }

// void Script::setGlobalString(const string &name, const string &value) {
//   lua_pushstring(lua_state_, value.c_str());
//   lua_setglobal(lua_state_, name.c_str());
// }

// double Script::getGlobalNumber(const string &name) {
//   double value = 0.0;
//   try {
//     lua_getglobal(lua_state_, name.c_str());
//     value = lua_tonumber(lua_state_, -1);
//     lua_pop(lua_state_, 1);
//   } catch(...) { }

//   return value;
// }

// void Script::setGlobalNumber(const string &name, const double &value) {
//     lua_pushnumber(lua_state_, (int)value);
//     lua_setglobal(lua_state_, name.c_str());
// }

// bool Script::getGlobalBoolean(const string &name) {
//   bool value = 0;

//   try {
//     lua_getglobal(lua_state_, name.c_str());
//     value = (bool) lua_toboolean(lua_state_, -1);
//     lua_pop(lua_state_, 1);
//   } catch(...) { }

//   return value;
// }

// void Script::setGlobalBoolean(const string &name, const bool &value) {
//   lua_pushboolean(lua_state_, (int)value);
//   lua_setglobal(lua_state_, name.c_str());
// }

// //call script function, 0 args, 0 retvals
// void Script::runFunction(const string &name) {
//   //call script function, 0 args, 0 retvals
//   lua_getglobal(lua_state_, name.c_str());
//   lua_call(lua_state_, 0, 0);
// }
