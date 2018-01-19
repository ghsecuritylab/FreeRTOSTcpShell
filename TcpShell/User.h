#pragma once

#include <FreeRTOS.h>
#include <semphr.h>
#include "tcpshell.h"

#define MAX_USERAPP 9

// Environment variable definitions
#define ENV_USERNAME "USER"
#define ENV_ERROR    "ERROR"

// User apps
extern const UserApp LogonApp;
extern const UserApp ShellApp;
extern const UserApp EchoApp;
extern const UserApp SetApp;
extern const UserApp HostnameApp;
extern const UserApp MacApp;
extern const UserApp ResetApp;
extern const UserApp ExitApp;
extern const UserApp HelpApp;

// The user app list. Last app token is NULL.
extern const PUserApp AppList[MAX_USERAPP];

extern SemaphoreHandle_t SystemSemaphore;
