#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
#include "stdio.h"
#include "string.h"

int main() {
    char buf[256];
    int error;
    lua_State *L = luaL_newstate(); // open Lua
    luaL_openlibs(L);               // open standard libraries
    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        error = luaL_loadstring(L, buf) || lua_pcall(L, 0, 0, 0);
        // loadstring compiles the entered line and pushes it into stack
        // pcall pops a function from the stack and runs in protected mode
        // both return 0 if no errors
        if (error) {
            fprintf(stderr, "%s\n", lua_tostring(L, -1));
            lua_pop(L, 1); // pop error message from the stack
        }
    }
    lua_close(L);

    return 0;
}