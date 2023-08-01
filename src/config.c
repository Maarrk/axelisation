#include "config.h"
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
#include <string.h>

int GetVector(lua_State *L, const char *path, Vector2 *outVector, char **outMessage);
int GetFloat(lua_State *L, const char *path, float *outFloat, char **outMessage);

const char *configScript = "src/config.lua";

Config DefaultConfig() {
    Config config;
    config.gravity = (Vector2){0.0f, 256.0f};
    config.playerWalkVelocity = 64.0f;
    config.playerJumpVelocity = (Vector2){0.0f, -128.0f};
    return config;
}

#define CONFIG_FLOAT_COUNT 1
#define CONFIG_VECTOR_COUNT 2

int LoadConfig(Config *outConfig, char **outMessage) {
    Config newConfig = DefaultConfig();

    // Manual reflection ;_;
    const struct {
        const char *path;
        float *field;
    } fieldsFloat[CONFIG_FLOAT_COUNT] = {
        {"config.playerWalkVelocity", &newConfig.playerWalkVelocity},
    };
    const struct {
        const char *path;
        Vector2 *field;
    } fieldsVector[CONFIG_VECTOR_COUNT] = {
        {"config.gravity", &newConfig.gravity},
        {"config.playerJumpVelocity", &newConfig.playerJumpVelocity},
    };

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    if (luaL_loadfile(L, configScript) || lua_pcall(L, 0, 0, 0)) {
        *outMessage = TextFormat("Cannot run Lua file '%s': %s", configScript, lua_tostring(L, -1));
        lua_close(L);
        return 1;
    }

    for (size_t i = 0; i < CONFIG_FLOAT_COUNT; i++) {
        if (GetFloat(L, fieldsFloat[i].path, fieldsFloat[i].field, outMessage)) {
            // *outMessage = TextFormat("Cannot get float at config path '%s'",
            // fieldsFloat[i].path);
            lua_close(L);
            return 1;
        }
    }
    for (size_t i = 0; i < CONFIG_VECTOR_COUNT; i++) {
        if (GetVector(L, fieldsVector[i].path, fieldsVector[i].field, outMessage)) {
            // *outMessage = TextFormat("Cannot get vector at config path '%s'",
            // fieldsVector[i].path);
            lua_close(L);
            return 1;
        }
    }

    *outConfig = newConfig;
    *outMessage = TextFormat("");
    return 0;
}

int GetVector(lua_State *L, const char *path, Vector2 *outVector, char **outMessage) {
    if (GetFloat(L, TextFormat("%s.x", path), &outVector->x, outMessage)) {
        return 1;
    }

    if (GetFloat(L, TextFormat("%s.y", path), &outVector->y, outMessage)) {
        return 1;
    }

    return 0;
}

int GetFloat(lua_State *L, const char *path, float *outFloat, char **outMessage) {
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
                *outMessage = TextFormat("%s '%s' is not a table", i == 0 ? "Global" : "Field",
                                         pathSegments[i]);
                return 1;
            }
        }
    }

    int isNumber;
    float variable = (float)lua_tonumberx(L, -1, &isNumber);
    if (!isNumber) {
        *outMessage = TextFormat("Variable at '%s' is not a number", path);
        return 1;
    }

    *outFloat = variable;
    return 0;
}
