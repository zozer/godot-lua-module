#ifndef LUA_HPP
#define LUA_HPP

#include <Godot.hpp>
#include <Node.hpp>
#include <map>

extern "C" {
    #include "luasrc/lua.h"
    #include "luasrc/lauxlib.h"
    #include "luasrc/lualib.h"
}

namespace godot {
    class LuaScript: public Node {
        GODOT_CLASS(LuaScript, Node)
    public:
        static void _register_methods();

        void _init();
        LuaScript();
        ~LuaScript();
        void printError(const std::string& variableName, const std::string& reason);

        bool load(String filename);
        Variant execute(String name, Array array);
        String getName();
        Variant test();
        void pushVariant(lua_State* L, Variant var);
        Variant popVariant(lua_State* L);
        Variant getVariant(lua_State* L, int index = -1);
    private:
        std::map<String, lua_State*> functions;
    };
 
}

#endif