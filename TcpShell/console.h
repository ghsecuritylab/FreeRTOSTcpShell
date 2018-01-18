#pragma once

#include <FreeRTOS.h>
#include <semphr.h>
#include "tcpshell.h"

#define MAX_USERAPP 9
#define MAX_USERNAME_LEN  128
#define MAX_ENV_LEN       32
#define MAX_ARGV          9
#define MAX_ARGUMENTS_LEN 200
#define MAX_USERENVIROMNENT_LEN 100

// Environment variable definitions
#define ENV_USERNAME "USER"
#define ENV_ERROR    "ERROR"

typedef enum ConsoleFlags
{
	ConsoleFlagsNone    = 0x0,
	ConsoleFlagsEchoOff = 0x1
} ConsoleFlags;

// These typedefs define the user and app context
typedef struct console_t console, *pconsole;

typedef int(*app_main)(pconsole, int argc, char** argv);

typedef struct app_t
{
	const char* name;
	app_main run;
	const char* usage;
	const char* description;
} app, *papp;

typedef struct console_env_t
{
	// Caller owns variable name
	char name[MAX_ENV_LEN];
	char value[MAX_USERENVIROMNENT_LEN];
	struct console_env_t* next;
} console_env, *pconsole_env;

typedef struct console_t
{
	// TCP/IP session stuff
	struct netconn *conn;
	unsigned int connid;
	bool exiting;
	
	// I/O related stuff
	struct telnet_t* telnet;
	struct netbuf* buf; // TCP input and buffer
	char* ptr;
	unsigned short remaining;
	int write_err;
	bool newch;
	
	// App stuff
	papp next_app;
	int argc;
	char* argv[MAX_ARGV];
	char command_line[MAX_ARGUMENTS_LEN];
	pconsole_env env;
} console, *pconsole;

// User apps
extern const app logon_app;
extern const app shell_app;
extern const app echo_app;
extern const app set_app;
extern const app hostname_app;
extern const app mac_app;
extern const app reset_app;
extern const app exit_app;
extern const app help_app;

// The user app list. Last app token is NULL.
extern const papp app_list[MAX_USERAPP];

// TCP/IP client session state stuff
pconsole console_init(struct netconn* conn, unsigned int connid);
void console_free(pconsole*);
int console_getchar(pconsole context);
int console_putchar(pconsole context, char ch);
int console_puts(pconsole context, const char* str);
int console_printf(pconsole context, const char* fmt, ...);
int console_vprintf(pconsole context, const char* fmt, va_list args);
int console_setflags(pconsole context, ConsoleFlags flags);
int console_unsetflags(pconsole context, ConsoleFlags flags);
int console_tokenize(pconsole context, const char* arguments);
int console_exec(pconsole context, const papp app);
int console_getenv(pconsole context, const char* name, const char** value);
int console_setenv(pconsole context, const char* name, char* value);
int console_unsetenv(pconsole context, const char* name);
int console_getline(pconsole context, ConsoleFlags flags, char* line, size_t len);
