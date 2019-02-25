#include "lua.hpp"

using namespace godot;

void LuaScript::_register_methods() {
    register_method("execute", &LuaScript::execute);
    register_method("load",&LuaScript::load);
    register_method("pushVariant",&LuaScript::pushGlobalVariant);
    //register_method("pushObject",&LuaScript::pushGlobalObject);
}

void LuaScript::_init() {
    L = luaL_newstate();
    luaL_openlibs(L);
}

LuaScript::LuaScript() {
}

LuaScript::~LuaScript() {
    lua_close(L);
}

bool LuaScript::load(String fileName) {
    File *file = File::_new();
    file->open(fileName,File::ModeFlags::READ);
    String text = file->get_as_text();
    PoolStringArray lines = text.split("\n");
    std::map<String,String> allFunctions;
    String inFunction = "";
    String functionBegin = "function";
    String functionEnd = "end";
    for (int i = 0; i < lines.size(); i++) {
        String line = lines[i];
        if (inFunction == "") {
            if (line.begins_with(functionBegin)) {
                inFunction = true;
                int endPos = line.find("(");
                String name = line.substr(functionBegin.length(),endPos-functionBegin.length()).strip_edges();               
                allFunctions.insert(std::pair<String,String>(name,line+"\n"));
                inFunction = name;
            }
        } else {
            allFunctions[inFunction]+=(line+"\n");
            if (line == functionEnd) {
                inFunction = "";
            }
        }
    }

    for(std::pair<String,String> function : allFunctions) {
        if (luaL_loadstring(L, function.second.alloc_c_string()) || (lua_pcall(L, 0, 0 , 0))) {
            Godot::print("Error: script not loaded (" + function.first + ")");
            L = 0;
            return false;
        }
    }
    file->close();
    return true;
}

Variant LuaScript::execute(String name, Array array) {
    lua_getglobal(L,name.alloc_c_string());
    for (int i = 0; i < array.size(); i++) {
        Variant var = array[i];
        pushVariant(var);
    }

    lua_pcall(L,array.size(),LUA_MULTRET,0);
    int numReturns = lua_gettop(L);

    if (numReturns) {
        Array results;
        for(int i = 0; i < numReturns; i++) {
            results.append(popVariant());
        }
        return results;
    } else {
        return Variant();
    }
}

bool LuaScript::pushVariant(Variant var) {
    switch (var.get_type())
    {
        case Variant::Type::STRING:
            lua_pushstring(L, (var.operator godot::String().alloc_c_string()));
            break;
        case Variant::Type::INT:
            lua_pushinteger(L, (int64_t)var);
            break;
        case Variant::Type::BOOL:
            lua_pushboolean(L, (bool)var);
            break;
        case Variant::Type::ARRAY: {
            Array array = var.operator godot::Array();
            lua_newtable(L);
            for(int i = 0; i < array.size(); i++) {
                Variant key = i+1;
                Variant value = array[i];
                pushVariant(key);
                pushVariant(value);
                lua_settable(L,-3);
            }
            break;
        }
        case Variant::Type::DICTIONARY:
            lua_newtable(L);
            for(int i = 0; i < ((Dictionary)var).size(); i++) {
                Variant key = ((Dictionary)var).keys()[i];
                Variant value = ((Dictionary)var)[key];
                pushVariant(key);
                pushVariant(value);
                lua_settable(L,-3);
            }
            break;
        default:
            Godot::print(var);
            return false;
    }
    return true;
}

bool LuaScript::pushGlobalVariant(Variant var, String name) {
    if (pushVariant(var)) {
        lua_setglobal(L,name.alloc_c_string());
        return true;
    }
    return false;
}

bool LuaScript::pushObject(Object obj) {
    return true;
}

bool LuaScript::pushGlobalObject(Object obj, String name) {
    bool ret = pushObject(obj);
    if (ret) {
        lua_setglobal(L, name.alloc_c_string());
    }
    return ret;
}

Variant LuaScript::popVariant() {
    Variant result = getVariant();
    lua_pop(L,1);
    return result;
}

Variant LuaScript::getVariant(int index) {
    Variant result;
    int type = lua_type(L,index);
    switch (type) {
        case LUA_TSTRING:
            result = lua_tostring(L,index);
            break;
        case LUA_TNUMBER:
            result = lua_tonumber(L,index);
            break;
        case LUA_TBOOLEAN:
            result = (bool)lua_toboolean(L,index);
            break;
        case LUA_TTABLE:
        {
            Dictionary dict;
            for (lua_pushnil(L); lua_next(L, index-1); lua_pop(L, 1)) {
                Variant key = getVariant(-2);
                Variant value = getVariant(-1);
                dict[key] = value;
            }
            result = dict;
            break;
        }
        default:
            //Godot::print(Variant(type));
            result = 0;
    }
    return result;
}

//lua functions
int LuaScript::testing(lua_State* L) {
    int num1 = lua_tonumber(L,1);
    int num2 = lua_tonumber(L,2);
    
    lua_pushnumber(L, num1+ num2);
    return 1;
}