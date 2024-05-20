#include "framework.h"
#include "luaTools.h"
#include "logger.h"
#include "utils.h"

void stackUpdate(lua_State *L, int &stackDepth, int initialStack)
{
    stackDepth = lua_gettop(L) - initialStack;
}

void stackPop(lua_State *L, int popDepth)
{
    lua_pop(L, popDepth);
}

void stackClean(lua_State *L, int stackDepth)
{
    lua_pop(L, stackDepth);
}

void luaLogTableKeys(lua_State *L, int index)
{
    if (lua_istable(L, index))
    {
        STACK_INIT;

        lua_pushvalue(L, index);
        lua_pushnil(L);
        while (lua_next(L, -2))
        {
            lua_pushvalue(L, -2);
            const char *key = lua_tostring(L, -1);
            log(key);
            lua_pop(L, 2);
        }
        lua_pop(L, 1);

        STACK_CLEAN;
    }
}

void luaTableToJSON(lua_State *L, int index, json &data, bool logKeys)
{
    if (lua_istable(L, index))
    {
        STACK_INIT;

        lua_pushvalue(L, index);
        lua_pushnil(L);
        while (lua_next(L, -2))
        {
            lua_pushvalue(L, -2);
            const char *key = lua_tostring(L, -1);
            if (logKeys)
            {
                log(key);
            }
            if (lua_istable(L, -2))
            {
                if (!json_has_object_field(data, key))
                    data[key] = json::object();
                luaTableToJSON(L, -2, data[key], logKeys);
            }
            else if (lua_isnumber(L, -2))
            {
                data[(key)] = json::number_float_t(lua_tonumber(L, -2));
            }
            else if (lua_isboolean(L, -2))
            {
                data[(key)] = json::boolean_t(lua_toboolean(L, -2));
            }
            else if (lua_isstring(L, -2)) // Keep last, lua_isstring only checks if it can be stringified (not if it actually IS a string)
            {
                data[(key)] = json::string_t(lua_tostring(L, -2));
            }
            lua_pop(L, 2);
        }
        lua_pop(L, 1);

        STACK_CLEAN;
    }
}
