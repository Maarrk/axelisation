#include "config.h"
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
#include <string.h>

int GetVector(lua_State *L, const char *path, Vector2 *outVector);
int GetFloat(lua_State *L, const char *path, float *outFloat);

Config DefaultConfig() {
    Config config;
    config.gravity = (Vector2){0.0f, 256.0f};
    config.playerWalkVelocity = 64.0f;
    config.playerJumpVelocity = (Vector2){0.0f, -128.0f};
    return config;
}

int LoadConfig(Config *outConfig) {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    const char *configScript = "src/config.lua";
    if (luaL_loadfile(L, configScript) || lua_pcall(L, 0, 0, 0)) {
        TraceLog(LOG_ERROR, "Cannot run Lua file '%s': %s", configScript, lua_tostring(L, -1));
        lua_close(L);
        return 1;
    }

    Config newConfig = DefaultConfig();

    if (GetFloat(L, "config.playerWalkVelocity", &newConfig.playerWalkVelocity)) {
        TraceLog(LOG_ERROR, "Cannot get float at config path '%s'", "config.playerWalkVelocity");
        lua_close(L);
        return 1;
    }

    if (GetVector(L, "config.gravity", &newConfig.gravity)) {
        TraceLog(LOG_ERROR, "Cannot get vector at config path '%s'", "config.playerWalkVelocity");
        lua_close(L);
        return 1;
    }

    if (GetVector(L, "config.playerJumpVelocity", &newConfig.playerJumpVelocity)) {
        TraceLog(LOG_ERROR, "Cannot get vector at config path '%s'", "config.playerWalkVelocity");
        lua_close(L);
        return 1;
    }

    *outConfig = newConfig;
    return 0;
}

int GetVector(lua_State *L, const char *path, Vector2 *outVector) {
    if (GetFloat(L, TextFormat("%s.x", path), &outVector->x)) {
        return 1;
    }

    if (GetFloat(L, TextFormat("%s.y", path), &outVector->y)) {
        return 1;
    }

    return 0;
}

int GetFloat(lua_State *L, const char *path, float *outFloat) {
    TraceLog(LOG_ERROR, "dupa");
    int segmentCount;
    char **pathSegments = TextSplit(path, '.', &segmentCount);

    for (size_t i = 0; i < segmentCount; i++) {
        if (i == 0) {
            lua_getglobal(L, pathSegments[i]);
        } else {
            lua_getfield(L, -1, pathSegments[i]);
        }

        if (i != segmentCount - 1) {
            if (!lua_istable(L, -1)) {
                TraceLog(LOG_ERROR, "%s '%s' is not a table", i == 0 ? "Global" : "Field",
                         pathSegments[i]);
                return 1;
            }
        }
    }

    int isNumber;
    float variable = (float)lua_tonumberx(L, -1, &isNumber);
    if (!isNumber) {
        TraceLog(LOG_ERROR, "Variable at '%s' is not a number", path);
        return 1;
    }

    *outFloat = variable;
    return 0;
}
