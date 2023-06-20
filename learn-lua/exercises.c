#include "ctype.h"
#include "getopt.h"
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
#include "stdio.h"
#include "string.h"

void basicInterpreter(lua_State *L);
void stackOperations(lua_State *L);
void stackDump(lua_State *L);

int main(int argc, char **argv) {
    char *evalue = NULL;
    int flag_char;
    while ((flag_char = getopt(argc, argv, "e:")) != -1) {
        switch (flag_char) {
        case 'e':
            evalue = optarg;
            break;

        case '?': {
            if (optopt == 'e')
                fprintf(stderr, "Option -%c requires an argument\n", optopt);
            else if (isprint(optopt))
                fprintf(stderr, "Unknown option -%c\n", optopt);
            else
                fprintf(stderr, "Unknown option characteer \\x%X\n", optopt);

            return 1;
        }

        default:
            return 2;
        }
    }

    typedef enum {
        EXERCISE_NONE,
        EXERCISE_27_1,
        EXERCISE_27_2,
        EXERCISE_27_3,
    } exercise_t;
    exercise_t selected;
    if (evalue == NULL)
        selected = EXERCISE_27_1; // default is interpreter
    else {
        if (strcmp(evalue, "27.1") == 0)
            selected = EXERCISE_27_1;
        else if (strcmp(evalue, "27.2"))
            selected = EXERCISE_27_2;
        else if (strcmp(evalue, "27.3"))
            selected = EXERCISE_27_3;
        else {
            fprintf(stderr, "Invalid exercise selected: '%s'\n", evalue);
            return 1;
        }
    }

    lua_State *L = luaL_newstate(); // open Lua
    luaL_openlibs(L);               // open standard libraries

    switch (selected) {
    case EXERCISE_27_1:
        basicInterpreter(L);
        break;

    case EXERCISE_27_2:
        stackOperations(L);
        break;

    case EXERCISE_27_3:
        stackOperations(L);
        stackDump(L);
        break;

    default:
        fprintf(stderr, "Unimplemented exercise_t %d\n", selected);
        break;
    }

    lua_close(L);

    return 0;
}

// Exercise 27.1
void basicInterpreter(lua_State *L) {
    char buf[256];
    int error;
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
}

// Exercise 27.2
void stackOperations(lua_State *L) {
    lua_pushnumber(L, 3.5);     // 3.5
    lua_pushstring(L, "hello"); // 3.5      hello
    lua_pushnil(L);             // 3.5      hello   nil
    lua_rotate(L, 1, -1);       // hello    nil     3.5
    lua_pushvalue(L, -2);       // hello    nil     3.5     nil
    lua_remove(L, 1);           // nil      3.5     nil
    lua_insert(L, -2);          // nil      nil     3.5
}

// Figure 27.2
void stackDump(lua_State *L) {
    int top = lua_gettop(L);
    for (size_t i = 1; i <= top; i++) {
        int t = lua_type(L, i);
        switch (t) {
        case LUA_TSTRING:
            printf("%s", lua_tostring(L, i));
            break;

        case LUA_TBOOLEAN:
            printf(lua_toboolean(L, i) ? "true" : "false");
            break;

        case LUA_TNUMBER: {
            if (lua_isinteger(L, i)) {
                printf("%lld", lua_tointeger(L, i));
            } else {
                printf("%g", lua_tonumber(L, i));
            }
            break;
        }

        default: // just print the type name
            printf("%s", lua_typename(L, i));
            break;
        }
    }
}