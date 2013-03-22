#ifndef SCRIPT_H
#define SCRIPT_H

#include <string>
#include "lua.hpp"

class Script
{
    public:
        Script();
        // Script(std::string scriptfile);
        virtual ~Script();

        bool loadScript(std::string scriptfile);

        std::string getGlobalString(std::string name);
        void setGlobalString(std::string name, std::string value);

        double getGlobalNumber(std::string name);
        void setGlobalNumber(std::string name, double value);

        bool getGlobalBoolean(std::string name);
        void setGlobalBoolean(std::string name, bool value);

        void runFunction(std::string name);

    private:
        lua_State *luaState;
};

#endif // SCRIPT_H
