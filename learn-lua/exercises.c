#include "ctype.h"
#include "float.h"
#include "getopt.h"
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
#include "math.h"
#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

// Exercise 27.4
typedef struct {
    lua_Alloc oldAllocFunction;
    void *oldUserData;
    size_t memoryLimit;
    // Signed in case it starts counting when something is already allocated
    long long memoryUsed;
} UserData;

void basicInterpreter(lua_State *L);
void stackOperations(lua_State *L, int dump);
void stackDump(lua_State *L);
void setLimit(lua_State *L, size_t maxMemory, UserData *outUserData);
void *limitedAllocFunction(void *ud, void *ptr, size_t osize, size_t nsize);
void error(lua_State *L, const char *fmt, ...);
void call_va(lua_State *L, const char *func, const char *sig, ...);
void plotFile(lua_State *L, const char *filename);

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
        EXERCISE_27_4,
        EXERCISE_28_1,
    } exercise_t;
    exercise_t selected;
    if (evalue == NULL)
        selected = EXERCISE_27_1; // default is interpreter
    else {
        if (strcmp(evalue, "27.1") == 0)
            selected = EXERCISE_27_1;
        else if (strcmp(evalue, "27.2") == 0)
            selected = EXERCISE_27_2;
        else if (strcmp(evalue, "27.3") == 0)
            selected = EXERCISE_27_3;
        else if (strcmp(evalue, "27.4") == 0)
            selected = EXERCISE_27_4;
        else if (strcmp(evalue, "28.1") == 0)
            selected = EXERCISE_28_1;
        else {
            fprintf(stderr, "Invalid exercise selected: '%s'\n", evalue);
            return 1;
        }
    }

    lua_State *L = luaL_newstate(); // open Lua
    luaL_openlibs(L);               // open standard libraries
    UserData allocFunctionUserData;

    switch (selected) {
    case EXERCISE_27_1:
        basicInterpreter(L);
        break;

    case EXERCISE_27_2:
        stackOperations(L, 1);
        break;

    case EXERCISE_27_3:
        stackOperations(L, 0);
        stackDump(L);
        break;

    case EXERCISE_27_4:
        setLimit(L, 1024, &allocFunctionUserData);
        basicInterpreter(L);
        break;

    case EXERCISE_28_1:
        plotFile(L, "learn-lua/plotting.lua");
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
void stackOperations(lua_State *L, int dump) {
    lua_pushnumber(L, 3.5);     // 3.5
    if (dump)                   //
        stackDump(L);           //
    lua_pushstring(L, "hello"); // 3.5      hello
    if (dump)                   //
        stackDump(L);           //
    lua_pushnil(L);             // 3.5      hello   nil
    if (dump)                   //
        stackDump(L);           //
    lua_rotate(L, 1, -1);       // hello    nil     3.5
    if (dump)                   //
        stackDump(L);           //
    lua_pushvalue(L, -2);       // hello    nil     3.5     nil
    if (dump)                   //
        stackDump(L);           //
    lua_remove(L, 1);           // nil      3.5     nil
    if (dump)                   //
        stackDump(L);           //
    lua_insert(L, -2);          // nil      nil     3.5
    if (dump)                   //
        stackDump(L);           //
}

// Figure 27.2
void stackDump(lua_State *L) {
    int top = lua_gettop(L);
    for (size_t i = 1; i <= top; i++) {
        int t = lua_type(L, i);
        switch (t) {
        case LUA_TSTRING:
            printf("'%s'", lua_tostring(L, i));
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
            printf("%s", lua_typename(L, t));
            break;
        }
        printf(" ");
    }
    printf("\n");
}

// Exercise 27.4
void setLimit(lua_State *L, size_t memoryLimit, UserData *outUserData) {
    outUserData->oldAllocFunction = lua_getallocf(L, &(outUserData->oldUserData));
    outUserData->memoryLimit = memoryLimit;
    outUserData->memoryUsed = 0; // Only now start tracking

    lua_setallocf(L, &limitedAllocFunction, outUserData);
}

void *limitedAllocFunction(void *ud, void *ptr, size_t osize, size_t nsize) {
    UserData *userData = (UserData *)ud;
    long long oldSize = (ptr == NULL) ? 0 : osize;
    long long sizeChange = nsize - oldSize;

    if (userData->memoryUsed + sizeChange > userData->memoryLimit) {
        // Can't allocate or relocate block due to memory limit
        return NULL;
    }

    userData->memoryUsed += sizeChange;
    return (*(userData->oldAllocFunction))(userData->oldUserData, ptr, osize, nsize);
}

// Exercise 28.1
void plotFile(lua_State *L, const char *filename) {
    if (luaL_loadfile(L, filename) || lua_pcall(L, 0, 0, 0)) // run the compiled chunk
        error(L, "cannot run file %s: %s", filename, lua_tostring(L, -1));

    const int columns = 80;
    const int rows = 25;

    double results[80];
    double min_result = DBL_MAX;
    double max_result = DBL_MIN;

    for (size_t i = 0; i < columns; i++) {
        double x = (double)i / (double)(columns - 1);
        call_va(L, "f", "d>d", x, &results[i]);

        if (results[i] < min_result)
            min_result = results[i];
        if (results[i] > max_result)
            max_result = results[i];
    }

    int result_rows[80];
    for (size_t i = 0; i < columns; i++) {
        result_rows[i] =
            rows - 1 - round((results[i] - min_result) / (max_result - min_result) * (rows - 1));
    }

    for (size_t r = 0; r < rows; r++) {
        for (size_t c = 0; c < columns; c++) {
            if (result_rows[c] == r) {
                printf("*");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
}

// Exercise 28.2
void call_va(lua_State *L, const char *func, const char *sig, ...) {
    va_list argp; // for iterating through additional arguments
    int narg;     // number of arguments
    int nres;     // number of results

    va_start(argp, sig);
    lua_getglobal(L, func); // push function to stack

    // push arguments to the function
    for (narg = 0; *sig; narg++) {
        // for each character in string sig get argument with va_arg

        luaL_checkstack(L, 1, "too many arguments");

        switch (*sig++) {
        case 'd': // double
            lua_pushnumber(L, va_arg(argp, double));
            break;

        case 'i': // int
            lua_pushinteger(L, va_arg(argp, int));
            break;

        case 's': // string
            lua_pushstring(L, va_arg(argp, char *));
            break;

        case '>':         // end of arguments
            goto endargs; // break out of the loop

        default:
            error(L, "invalid argument option (%c)", *(sig - 1)); // sig is already incremented
        }
    }
endargs:

    nres = strlen(sig); // what is still left after iterating are results

    if (lua_pcall(L, narg, nres, 0) != 0) { // do the actual call here
        error(L, "error calling '%s': %s", func, lua_tostring(L, -1));
    }

    int res_i = -nres; // stack index of the first result
    while (*sig) {     // go through the rest of the signature
        switch (*sig++) {
        case 'd': { // double
            int isnum;
            double n = lua_tonumberx(L, res_i, &isnum);
            if (!isnum)
                error(L, "wrong type of result %d, expected number", 1 + nres + res_i);
            *va_arg(argp, double *) = n;
            break;
        }

        case 'i': { // int
            int isnum;
            double n = lua_tointegerx(L, res_i, &isnum);
            if (!isnum)
                error(L, "wrong type of result %d, expected integer", 1 + nres + res_i);
            *va_arg(argp, int *) = n;
            break;
        }
        case 's': { // string
            const char *s = lua_tostring(L, res_i);
            if (s == NULL)
                error(L, "wrong type of result %d, expected string", 1 + nres + res_i);
            *va_arg(argp, const char **) = s;
            break;
        }

        default:
            error(L, "invalid return option (%c)", *(sig - 1)); // sig is already incremented
        }
    }

    va_end(argp);
}

void error(lua_State *L, const char *fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    va_end(argp);
    lua_close(L);
    exit(EXIT_FAILURE);
}