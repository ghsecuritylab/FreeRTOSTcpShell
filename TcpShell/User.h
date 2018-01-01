#pragma once

// User (private) functionality. Defines user apps and general context

#define USER_NAME_LEN 64

typedef struct UserContext_t UserContext, *PUserContext;
typedef struct AppContext_t *PAppContext;

typedef bool(*PAppRun)(PUserContext);
typedef void(*PAppTerminate)(PUserContext);
typedef PAppContext(*PAppCreateContext)(PUserContext);
typedef void(*PAppDeleteContext)(PAppContext);

typedef struct UserApp_t
{
	const char* AppName;
	PAppRun Run;
	PAppTerminate Terminate;
	PAppCreateContext CreateContext;
	PAppDeleteContext DeleteContext;
} UserApp, *PUserApp;

typedef struct EnvVariable_t
{
	const char* Name;
	char* buf;
	size_t buflen;
	struct EnvVariable_t* Next;
} EnvVariable, *PEnvVariable;

typedef struct UserContext_t
{
	PUserApp CurrentApp;
	PAppContext AppContext;
	PEnvVariable FirstEnvVariable;
	QueueHandle_t OutQ;
	QueueHandle_t InQ;
	osThreadId ThreadId;
	bool Active;
	bool AppRunning;
	bool EchoOn;
} UserContext, *PUserContext;

extern PUserContext* Contexts;
extern int NumUserContexts;

// User apps
extern UserApp LogonApp;

// Standard environment variables
#define ENV_USERNAME "USER"

// Context functions. In most cases these are supposed to be called from a user app's run function
bool UserContextRunApp(PUserContext UserContext, const PUserApp App);
bool UserContextTerminateApp(PUserContext UserContext);
char UserContextGetChar(PUserContext UserContext);
bool UserContextPutChar(PUserContext UserContext, char ch);
int UserContextGetChars(PUserContext UserContext, char* buf, size_t buflen, char optTerminator);
int UserContextPutChars(PUserContext UserContext, const char* buf, size_t buflen);
int UserContextPutString(PUserContext UserContext, const char* buf);
bool UserContextSetEnv(PUserContext UserContext, const char* name, const char* buf, size_t buflen);
bool UserContextGetEnv(PUserContext UserContext, const char* name, const char** buf, size_t* buflen);
