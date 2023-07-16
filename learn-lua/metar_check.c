#include "ctype.h"
#include "getopt.h"
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

// #define CONFIG_GLOBALS 1
#define CONFIG_TABLE 1
// #define CONFIG_FUNCTION 1

void errorWithLua(lua_State *L, const char *fmt, ...);
void callLuaFunction(lua_State *L, const char *function_name, const char *signature, ...);
void getMetar(const char *url, char *output_buffer, size_t output_length);
void metarUrl(const char *airport, char *output_buffer, size_t output_length);

int main(int argc, char **argv) {
    {
        static struct option long_options[] = {
            {"help", 0, NULL, 'h'}, // clang doesn't accept const int no_argument, this is the 0
            {0, 0, 0, 0},
        };

        int c;

        while (1) {
            int option_index = 0;
            c = getopt_long(argc, argv, "h", long_options, &option_index);

            /* Detect the end of the options. */
            if (c == -1)
                break;

            switch (c) {
            case 0:
            case 'h':
                printf("Check current METAR for EPWA, EPMO and EPKK.\n"
                       "Depends on curl being available in path\n\n"
                       "Options:\n"
                       "  -h --help\tShow this screen.\n");
                exit(EXIT_SUCCESS);

            case '?':
                if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                exit(EXIT_FAILURE);

            default:
                fprintf(stderr, "getopt error");
                exit(EXIT_FAILURE);
            }
        }
    }

    char output_buffer[256];
    const char *airports[3] = {
        "EPWA",
        "EPMO",
        "EPKK",
    };
    for (size_t i = 0; i < 3; i++) {
        metarUrl(airports[i], output_buffer, sizeof(output_buffer));
        getMetar(output_buffer, output_buffer, sizeof(output_buffer));
        printf("%s\n", output_buffer);
    }

    return EXIT_SUCCESS;
}

void getMetar(const char *url, char *output_buffer, size_t output_length) {
    char command[256];
    int res = snprintf(command, sizeof(command), "curl -s %s", url);
    if (res < 0) {
        fprintf(stderr, "URL too long");
        exit(EXIT_FAILURE);
    }

    FILE *cmd = popen(command, "r");
    char result[1024];
    int read_length = fread(result, 1, sizeof(result), cmd);
    if (read_length == sizeof(result)) {
        fprintf(stderr, "Response too long");
        exit(EXIT_FAILURE);
    }
    pclose(cmd);

    char *metar_start = strstr(result, "METAR EP");
    if (metar_start == NULL) {
        fprintf(stderr, "Didn't find message start in curl response");
        exit(EXIT_FAILURE);
    }
    char *metar_end = strchr(metar_start, '=');
    if (metar_end == NULL) {
        fprintf(stderr, "Didn't find message end in curl response");
        exit(EXIT_FAILURE);
    }
    size_t metar_length = metar_end - metar_start + 1; // Include the terminating '='
    if (metar_length + 1 > output_length) {
        fprintf(stderr, "Message too long for buffer provided");
        exit(EXIT_FAILURE);
    }

    memcpy(output_buffer, metar_start, metar_length);
    output_buffer[metar_length] = '\0';
}

void metarUrl(const char *airport, char *output_buffer, size_t output_length) {
    lua_State *L = luaL_newstate(); // open Lua
    luaL_openlibs(L);               // open standard libraries

    const char *config_file = "learn-lua/metar_url.lua";
    if (luaL_loadfile(L, config_file) || lua_pcall(L, 0, 0, 0)) // run the compiled chunk
        errorWithLua(L, "Cannot run Lua file %s: %s", config_file, lua_tostring(L, -1));

#ifdef CONFIG_GLOBALS
    lua_getglobal(L, airport);
    const char *s = lua_tostring(L, -1);
    if (s == NULL)
        errorWithLua(L, "Could not load global string '%s'", airport);

    strcpy_s(output_buffer, output_length, s);
#else // CONFIG_GLOBALS
#ifdef CONFIG_TABLE
    const char *table_name = "airport_url";
    lua_getglobal(L, table_name);
    if (!lua_istable(L, -1))
        errorWithLua(L, "Global variable '%s' is not a table", table_name);

    lua_getfield(L, -1, airport);
    const char *s = lua_tostring(L, -1);
    if (s == NULL)
        errorWithLua(L, "Invalid field '%s' in table '%s'", airport, table_name);

    strcpy_s(output_buffer, output_length, s);
#else // CONFIG_TABLE
#ifdef CONFIG_FUNCTION
    char *s;
    callLuaFunction(L, "airport_to_url", "s>s", airport, &s);

    strcpy_s(output_buffer, output_length, s);
#else
#error "Choose an implementation from CONFIG_GLOBALS, CONFIG_TABLE, CONFIG_FUNCTION"
    snprintf(output_buffer, output_length, "https://awiacja.imgw.pl/metar00.php?airport=%s",
             airport);
#endif // CONFIG_FUNCTION
#endif // CONFIG_TABLE
#endif // CONFIG_GLOBALS
}

void errorWithLua(lua_State *L, const char *fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    va_end(argp);
    lua_close(L);
    exit(EXIT_FAILURE);
}

void callLuaFunction(lua_State *L, const char *function_name, const char *signature, ...) {
    va_list argp; // for iterating through additional arguments
    int narg;     // number of arguments
    int nres;     // number of results

    va_start(argp, signature);
    lua_getglobal(L, function_name); // push function to stack

    // push arguments to the function
    for (narg = 0; *signature; narg++) {
        // for each character in string signature get argument with va_arg

        luaL_checkstack(L, 1, "too many arguments");

        int finished = 0;
        switch (*signature++) {
        case 'b': // bool
            lua_pushboolean(L, va_arg(argp, int));
            break;

        case 'd': // double
            lua_pushnumber(L, va_arg(argp, double));
            break;

        case 'i': // int
            lua_pushinteger(L, va_arg(argp, int));
            break;

        case 's': // string
            lua_pushstring(L, va_arg(argp, char *));
            break;

        case '>': // end of arguments
            finished = 1;
            break;

        default:
            errorWithLua(L, "invalid argument option (%c)",
                         *(signature - 1)); // signature is already incremented
        }

        if (finished)
            break;
    }

    nres = strlen(signature); // what is still left after iterating are results

    if (lua_pcall(L, narg, nres, 0) != 0) { // do the actual call here
        errorWithLua(L, "Error calling '%s': %s", function_name, lua_tostring(L, -1));
    }

    int res_i = -nres;   // stack index of the first result
    while (*signature) { // go through the rest of the signature
        switch (*signature++) {
        case 'b': { // bool
            int b = lua_toboolean(L, res_i);
            *va_arg(argp, int *) = b;
            break;
        }

        case 'd': { // double
            int isnum;
            double n = lua_tonumberx(L, res_i, &isnum);
            if (!isnum)
                errorWithLua(L, "wrong type of result %d, expected number", 1 + nres + res_i);
            *va_arg(argp, double *) = n;
            break;
        }

        case 'i': { // int
            int isnum;
            double n = lua_tointegerx(L, res_i, &isnum);
            if (!isnum)
                errorWithLua(L, "wrong type of result %d, expected integer", 1 + nres + res_i);
            *va_arg(argp, int *) = n;
            break;
        }
        case 's': { // string
            const char *s = lua_tostring(L, res_i);
            if (s == NULL)
                errorWithLua(L, "wrong type of result %d, expected string", 1 + nres + res_i);
            *va_arg(argp, const char **) = s;
            break;
        }

        default:
            errorWithLua(L, "invalid return option (%c)",
                         *(signature - 1)); // signature is already incremented
        }
        res_i++;
    }

    va_end(argp);
}