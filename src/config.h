#ifndef CONFIG_H
#define CONFIG_H

#include "raylib.h"

typedef struct Config {
    Vector2 gravity;
    float playerWalkVelocity;
    Vector2 playerJumpVelocity;
} Config;

Config DefaultConfig();
int LoadConfig(Config *outConfig, char **outMessage);

extern const char *configScript;

#endif // CONFIG_H
