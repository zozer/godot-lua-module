#include "lua.hpp"

using namespace godot;

void LuaScript::_register_methods() {
    register_method("execute", &LuaScript::execute);
    register_method("load",&LuaScript::load);
    register_method("test",&LuaScript::test);
}

void LuaScript::_init() {

}

LuaScript::LuaScript() {
}

LuaScript::~LuaScript() {
    if(functions.size() > 0) {   
        for(const auto& pair: functions)
        {
            lua_close(pair.second);
        }      
    } 
}

void LuaScript::printError(const std::string& variableName, const std::string& reason) {
    //std::cout<<"Error: can't get ["<<variableName<<"]. "<<reason<<std::endl;
}

bool LuaScript::load(String filename) {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    String functionName = "";
    if (luaL_loadfile(L, filename.alloc_c_string()) || (lua_pcall(L, 0, 0 , 0))) {
        //std::cout<<"Error: script not loaded ("<<filename<<")"<<std::endl;
        L = 0;
        return false;
    }
    int posStart = filename.rfind('\\') + 1;
    int posEnd = filename.find('.',0);
    functionName = filename.substr(posStart,posEnd - posStart);
    functions.insert(std::pair<String,lua_State*>(functionName,L));
    return true;
}

Variant LuaScript::execute(String name, Array array) {
    if (!functions.count(name)) {
        return false;
    }
    lua_State* L = functions[name];
    lua_getglobal(L,name.alloc_c_string());
    for (int i = 0; i < array.size(); i++) {
        Variant var = array[i];
        pushVariant(L,var);
    }

    lua_pcall(L,array.size(),LUA_MULTRET,0);
    int numReturns = lua_gettop(L);

    if (numReturns) {
        Array results;
        for(int i = 0; i < numReturns; i++) {
            Variant result = popVariant(L);
        }
        return results;
    } else {
        return NULL;
    }
}

Variant LuaScript::test() {
    return get_parent()->get("data");
}

void LuaScript::pushVariant(lua_State* L, Variant var) {
    switch (var.get_type())
    {
        case Variant::Type::STRING:
            lua_pushstring(L, (var.operator godot::String().alloc_c_string()));
            break;
        case Variant::Type::INT:
            lua_pushinteger(L, var);
            break;
        case Variant::Type::DICTIONARY:
            lua_newtable(L);
            for(int i = 0; i < ((Dictionary)var).size(); i++) {
                Variant key = ((Dictionary)var).keys()[i];
                Variant value = ((Dictionary)var)[key];
                pushVariant(L,key);
                pushVariant(L,value);
                lua_settable(L,-3);
            }
            break;
        default:
            break;
    }
}

Variant LuaScript::popVariant(lua_State* L) {
    Variant result = getVariant(L);
    lua_pop(L,1);
    return result;
}

Variant LuaScript::getVariant(lua_State* L, int index) {
    Variant result;
    int type = lua_type(L,index);
    switch (type) {
        case LUA_TSTRING:
            result = lua_tostring(L,index);
            break;
        case LUA_TNUMBER:
            result = lua_tonumber(L,index);
            break;
        case LUA_TTABLE:
        {
            Dictionary dict;
            for (lua_pushnil(L); lua_next(L, index-1); lua_pop(L, 1)) {
                Variant key = getVariant(L, -2);
                Variant value = getVariant(L, -1);
                dict[key] = value;
            }
            lua_pop(L, 1);
            result = dict;
            break;
        }
        default:
            result = 0;
    }
    return result;
}