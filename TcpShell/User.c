#include <stm32f7xx_hal.h>
#include <../CMSIS_RTOS/cmsis_os.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <queue.h>
#include <stdarg.h>
#include "tcpshell.h"
#include "User.h"

#define STREAM_BUFSZ (1 << 8)

PUserContext* Contexts = NULL;
int NumUserContexts = 0;

void UserInit(int maxConns)
{
	assert(maxConns > 0);
	size_t contextSz = maxConns;
	Contexts = malloc(contextSz);
	if (Contexts)
	{
		NumUserContexts = maxConns;
		memset(Contexts, 0, contextSz);
	}
}

bool UserSessionLoad(int sessionId)
{
	assert(sessionId >= 0 && Contexts[sessionId] == NULL);
	if (sessionId >= 0 && Contexts[sessionId] == NULL)
	{
		Contexts[sessionId] = malloc(sizeof(UserContext));
		if (Contexts[sessionId])
		{
			memset(Contexts[sessionId], 0, sizeof(UserContext));
			Contexts[sessionId]->Active = true;
			Contexts[sessionId]->InQ = xQueueCreate(STREAM_BUFSZ, 1);
			Contexts[sessionId]->OutQ = xQueueCreate(STREAM_BUFSZ, 1);
			UserContextRunApp(Contexts[sessionId], &LogonApp);
			return true;
		}
	}
	
	return false;
}

bool UserSessionUnload(int sessionId)
{
	assert(sessionId >= 0 && Contexts[sessionId] != NULL);
	if (sessionId >= 0 && Contexts[sessionId] != NULL)
	{
		// Unload the current app
		if (Contexts[sessionId]->CurrentApp)
		{
			UserContextTerminateApp(Contexts[sessionId]);	
		}
		
		// Free the environment block...
		PEnvVariable CurEnvVariable = Contexts[sessionId]->FirstEnvVariable;
		while (CurEnvVariable)
		{
			if (CurEnvVariable->buf)
			{
				free(CurEnvVariable->buf);
			}
			
			CurEnvVariable = CurEnvVariable->Next;
		}
		
		// Free the in and out Q's
		vQueueDelete(Contexts[sessionId]->InQ);
		vQueueDelete(Contexts[sessionId]->OutQ);
		free(Contexts[sessionId]);
		return true;
	}
	
	return false;
}

_Bool UserSessionActive(int sessionId)
{
	assert(sessionId >= 0 && Contexts[sessionId] != NULL);
	if (sessionId >= 0 && Contexts[sessionId] != NULL)
	{
		return Contexts[sessionId]->Active;
	}
	
	return false;
}

_Bool UserSessionPutChar(int sessionId, char ch)
{
	assert(sessionId >= 0 && Contexts[sessionId] != NULL && Contexts[sessionId]->CurrentApp != NULL);
	if (sessionId >= 0 && Contexts[sessionId] != NULL && Contexts[sessionId]->CurrentApp != NULL)
	{
		// If an app doesn't respond in a minute, it is "too busy" so we request the session be cleaned up.
		return xQueueSend(Contexts[sessionId]->InQ, &ch, 60000 / portTICK_PERIOD_MS) == pdTRUE;
	}
	
	return false;
}

char UserSessionGetChar(int sessionId)
{
	assert(sessionId >= 0 && Contexts[sessionId] != NULL && Contexts[sessionId]->CurrentApp != NULL);
	if (sessionId >= 0 && Contexts[sessionId] != NULL && Contexts[sessionId]->CurrentApp != NULL)
	{
		char ch;
		if (xQueueReceive(Contexts[sessionId]->OutQ, &ch, 0) == pdTRUE)
		{
			return ch;
		}
	}
	
	return 0;
}

void UserAppThread(void const* argument)
{
	assert(argument);
	if (argument)
	{
		PUserContext UserContext = (PUserContext)argument;
		UserContext->CurrentApp->Run(UserContext);
		UserContext->AppRunning = false;
	}
}

bool UserContextRunApp(PUserContext UserContext, const PUserApp App)
{
	assert(UserContext && UserContext->CurrentApp);
	if (UserContext && UserContext->CurrentApp)
	{
		if (UserContext->CurrentApp)
		{
			// Whatever's running gets blown away...
			UserContextTerminateApp(UserContext);	
		}
		
		UserContext->CurrentApp = App;
		UserContext->AppRunning = true;
		UserContext->AppContext = App->CreateContext(UserContext);
		// Start the run thread for the app...
		osThreadDef(UserApp, UserAppThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 5);
		UserContext->ThreadId = osThreadCreate(osThread(UserApp), UserContext);
		return UserContext->ThreadId != NULL;
	}
	
	return false;
}

bool UserContextTerminateApp(PUserContext UserContext)
{
	assert(UserContext && UserContext->CurrentApp);
	if (UserContext && UserContext->CurrentApp)
	{
		UserContext->CurrentApp->Terminate(UserContext);
		if (UserContext->ThreadId != NULL && UserContext->ThreadId != osThreadGetId())
		{
			int i;
			for (i = 0; i < 60000 / 250; ++i)
			{
				// Cooperatively wait for the app to finish and the thread to join us...
				osDelay(250);
				if (!UserContext->AppRunning)
				{
					break;
				}
			}
		
			if (UserContext->AppRunning)
			{
				osThreadTerminate(UserContext->ThreadId);
			}
		}
		
		// Now delete the app context
		UserContext->CurrentApp->DeleteContext(UserContext->AppContext);		
		UserContext->ThreadId = NULL;
		UserContext->CurrentApp = NULL;
		UserContext->AppContext = NULL;
		return true;
	}
	
	return false;
}

char UserContextGetChar(PUserContext UserContext)
{
	assert(UserContext);
	if (UserContext)
	{
		char ch;
		if (xQueueReceive(UserContext->InQ, &ch, 0) == pdTRUE)
		{
			if (UserContext->EchoOn)
			{
				xQueueSend(UserContext->InQ, &ch, 250 / portTICK_PERIOD_MS);
			}
			
			return ch;
		}
		
	}
	
	return 0;
}

bool UserContextPutChar(PUserContext UserContext, char ch)
{
	assert(UserContext);
	if (UserContext)
	{
		// If an app doesn't respond in a minute, it is "too busy" so we request the session be cleaned up.
		return xQueueSend(UserContext->OutQ, &ch, 60000 / portTICK_PERIOD_MS) == pdTRUE;
	}
	
	return false;
}

int UserContextPutChars(PUserContext UserContext, const char* str, size_t buflen)
{
	int written = 0;
	assert(UserContext && str);
	if (UserContext && str)
	{
		while (written < buflen)
		{
			if (!UserContextPutChar(UserContext, str[written]))
			{
				break;
			}
			
			++written;
		}
	}
	
	return written;
}

int UserContextPutString(PUserContext UserContext, const char* buf)
{
	return UserContextPutChars(UserContext, buf, strlen(buf));
}

int UserContextGetChars(PUserContext UserContext, char* buf, size_t buflen, char optTerminator)
{
	int read = 0;
	assert(UserContext && buf);
	if (UserContext && buf)
	{
		char* ch = buf;
		while ((ch - buf) < buflen)
		{
			char rd;
			if (!(rd = UserContextGetChar(UserContext)) || rd == optTerminator)
			{
				break;
			}
			
			++read;
			*ch = rd;
			++ch;
		}
	}
	
	return read;
}

bool UserContextSetEnv(PUserContext UserContext, const char* name, const char* buf, size_t buflen)
{
	// Either this is a "new" variable and we need to set it, or a variable to update
	PEnvVariable FoundIt = NULL;
	assert(UserContext && name && buflen);
	if (UserContext && name && buflen)
	{
		FoundIt = UserContext->FirstEnvVariable;
		while (FoundIt)
		{
			if (strcmp(name, FoundIt->Name) == 0)
			{
				goto update;
			}
			
			FoundIt = FoundIt->Next;
		}
	}

update:
	if (name && buflen)
	{
		if (!FoundIt)
		{
			// So we have to add a new one...
			if(UserContext->FirstEnvVariable)
			{
				FoundIt = UserContext->FirstEnvVariable;
				while (FoundIt->Next)
				{
					FoundIt = FoundIt->Next;
				}
			
				FoundIt->Next = malloc(sizeof(EnvVariable));
				FoundIt = FoundIt->Next;
			}
			else
			{
				UserContext->FirstEnvVariable = FoundIt = malloc(sizeof(EnvVariable));
			}

			if (FoundIt)
			{
				memset(FoundIt, 0, sizeof(EnvVariable));
			}
		}
	
		if (FoundIt)
		{
			if (FoundIt->buf)
			{
				free(FoundIt->buf);
			}
		
			FoundIt->buf = malloc(buflen);
			if (FoundIt->buf)
			{
				memcpy(FoundIt->buf, buf, buflen);
				FoundIt->buflen = buflen;
			}
			else
			{
				FoundIt->buflen = 0;
			}
		
			return true;
		}
	}
	
	return false;
}

bool UserContextGetEnv(PUserContext UserContext, const char* name, const char** buf, size_t* buflen)
{
	PEnvVariable FoundIt = NULL;
	assert(UserContext && name && buf && buflen);
	if (UserContext && name && buf && buflen)
	{
		PEnvVariable FoundIt = NULL;
		*buf = NULL;
		*buflen = 0;
		FoundIt = UserContext->FirstEnvVariable;
		while (FoundIt)
		{
			if (strcmp(name, FoundIt->Name) == 0)
			{
				*buf = FoundIt->buf;
				*buflen = FoundIt->buflen;
				return true;
			}
			
			FoundIt = FoundIt->Next;
		}
	}
	
	return false;
}
