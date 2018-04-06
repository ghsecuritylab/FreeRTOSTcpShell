#include <../CMSIS_RTOS/cmsis_os.h>
#include <stm32f7xx_hal.h>
#include <lwip/api.h>
#include <lwip/opt.h>
#include <lwip/dhcp.h>
#include <lwip/tcpip.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <queue.h>
#include <stdarg.h>
#include "tcpshell.h"
#include "ethernetif.h"
#include <stdbool.h>

const papp app_list[MAX_USERAPP] = 
{ 
	(const papp)&logon_app,
	(const papp)&shell_app,
	(const papp)&echo_app,
	(const papp)&set_app,
	(const papp)&hostname_app,
	(const papp)&mac_app,
	(const papp)&reset_app,
	(const papp)&exit_app,
	(const papp)&help_app,
	(const papp)&beep_app,
	(const papp)&message_app,
	(const papp)&db_app
};

pconsole console_init(struct netconn* newconn, unsigned int connid)
{
	pconsole newConsole = pvPortMalloc(sizeof(console));
	if (newConsole)
	{
		memset(newConsole, 0, sizeof(console));
		newConsole->conn = newconn;
		newConsole->connid = connid;
	}
	
	return newConsole;
}

void console_free(pconsole* pContext)
{
	if (pContext)
	{
		if ((*pContext)->telnet)
		{
			telnet_free((*pContext)->telnet);
		}
		
		if ((*pContext)->conn)
		{
			if (netconn_err((*pContext)->conn) == ERR_OK)
			{
				// Only close if the conn is still open
				netconn_close((*pContext)->conn);	
			}
		
			netconn_delete((*pContext)->conn);
		}
	
		if ((*pContext)->buf)
		{
			netbuf_free((*pContext)->buf);
		}
	
		if ((*pContext)->env)
		{
			pconsole_env cur = (*pContext)->env;
			while (cur)
			{
				pconsole_env next = cur->next;
				vPortFree(cur);
				cur = next;
			}
		}
	
		vPortFree(*pContext);
		*pContext = NULL;
	}
}

int console_getchar(pconsole context)
{
	assert_if(context)
	{
		for (;;)
		{
			// Purge the current buf until we get a new non-control char
			context->newch = false;
			while (context->buf && context->remaining > 0)
			{
				char ch = *context->ptr++;
				--context->remaining;
				telnet_recv(context->telnet, &ch, 1);
				if (context->newch)
				{
					return ch;	
				}
			}
			
			// Uh oh. We purged our buf but didn't get a non-control char yet.
			err_t err;
			if (context->buf)
			{
				netbuf_delete(context->buf);
				context->buf = NULL;
			}
		
			// Read more...
			err = netconn_recv(context->conn, &context->buf);
			if (err == ERR_OK) 
			{
				netbuf_data(context->buf, (void**)&context->ptr, &context->remaining);
			}
			else
			{
				if (ERR_IS_FATAL(err))
				{
					dprintf("%d: netconn_recv failed: %d (%s)\n", context->connid, err, lwip_strerr(err));	
				}
			
				return err;
			}
		}
		
		assert(false);
	}
	
	return 0;
}

int console_putchar(pconsole context, char ch)
{
	assert_if(context)
	{
		context->write_err = 0;
		telnet_send(context->telnet, (const char*)&ch, 1);
		return context->write_err;
	}
	
	return 0;
}

int console_puts(pconsole context, const char* str)
{
	assert_if(context)
	{
		context->write_err = 0;
		telnet_send(context->telnet, str, strlen(str));
		return context->write_err;
	}
	
	return 0;
}

int console_printf(pconsole context, const char* fmt, ...)
{
	assert_if(context)
	{
		va_list args;
		va_start(args, fmt);
		int rc = console_vprintf(context, fmt, args);
		va_end(args);
		return rc;
	}
	
	return 0;
}

int console_vprintf(pconsole context, const char* fmt, va_list args)
{
	assert_if(context)
	{
		context->write_err = 0;
		int written = telnet_vprintf(context->telnet, fmt, args);
		if (context->write_err == 0)
		{
			return written;
		}
		
		return context->write_err;
	}
	
	return 0;
}

int console_setflags(pconsole context, ConsoleFlags flags)
{
	assert_if(context)
	{
		context->write_err = 0;
		if (flags & ConsoleFlagsEchoOff)
		{
			telnet_negotiate(context->telnet, TELNET_WILL, TELNET_TELOPT_ECHO);
			telnet_negotiate(context->telnet, TELNET_WILL, TELNET_TELOPT_SGA);
		}
		
		return context->write_err;
	}
	
	return 0;
}

int console_unsetflags(pconsole context, ConsoleFlags flags)
{
	assert_if(context)
	{
		context->write_err = 0;
		if (flags & ConsoleFlagsEchoOff)
		{
			telnet_negotiate(context->telnet, TELNET_WONT, TELNET_TELOPT_ECHO);
			telnet_negotiate(context->telnet, TELNET_WONT, TELNET_TELOPT_SGA);
		}
		
		return context->write_err;
	}
	
	return 0;
}

int console_tokenize(pconsole context, const char* arguments)
{
	// Very rudimentary console argument parser...
	int rc = 0;
	int argv_i;
	int start;
	int ch_i;
	size_t sizeArguments;
	bool escapeNext = false;
	assert_if(context && arguments)
	{
		if (!strncpy(context->command_line, arguments, sizeof(context->command_line)))
		{
			rc = -1;
			goto done;
		}
		
		sizeArguments = strlen(arguments);
		start = argv_i = ch_i = 0;
		for (; argv_i < MAX_ARGUMENTS_LEN && ch_i < sizeArguments; ++ch_i)
		{
			if (context->command_line[ch_i] == '^')
			{
				escapeNext = true;
			}
			else
			{
				if (escapeNext)
				{
					escapeNext = false;
				}
				else if (context->command_line[ch_i] == ' ')
				{
					while (ch_i < sizeArguments && context->command_line[ch_i] == ' ')
					{
						context->command_line[ch_i++] = 0;
					}
				
					if (argv_i < MAX_ARGUMENTS_LEN)
					{
						context->argv[argv_i++] = &context->command_line[start];
					}
				
					if (ch_i < sizeArguments)
					{
						start = ch_i;
					}
				}
			}
		}
		
		if (argv_i >= MAX_ARGUMENTS_LEN)
		{
			rc = -1;
			goto done;
		}
		
		if (argv_i < MAX_ARGUMENTS_LEN)
		{
			// Any remaining goes to argv_i
			context->argv[argv_i++] = &context->command_line[start];
		}
		
		context->argc = argv_i;
	}
	
done:
	if (rc != 0)
	{
		memset(context->command_line, 0, sizeof(context->command_line));
		memset(context->argv, 0, sizeof(context->argv));
		context->argc = 0;
		return rc;
	}
	
	return ch_i;
}

int console_exec(pconsole context, const papp app)
{
	assert_if(context && app)
	{
		assert_if(!context->next_app)
		{
			context->next_app = app;
		}
	}
	
	return 0;
}

int console_getenv(pconsole context, const char* name, const char** value)
{
	pconsole_env cur = context->env;
	assert_if(context && name && value)
	{
		while (cur)
		{
			if (strcmp(cur->name, name) == 0)
			{
				*value = cur->value;
				return 0;
			}
			
			cur = cur->next;
		}
		
		return -1;
	}
	
	return 0;
}

int console_setenv(pconsole context, const char* name, char* value)
{
	pconsole_env cur = context->env;
	assert_if(context && name)
	{
		if (!value)
		{
			return console_unsetenv(context, name);
		}
		
		while (cur)
		{
			if (strcmp(cur->name, name) == 0)
			{
				strncpy(cur->value, value, sizeof(cur->value));
				return 0;
			}
			
			cur = cur->next;
		}
		
		pconsole_env first = pvPortMalloc(sizeof(console_env));
		if (first)
		{
			// Prepend it to the environment block
			memset(first, 0, sizeof(console_env));	
			strncpy(first->name, name, sizeof(first->name));
			strncpy(first->value, value, sizeof(first->value));
			first->next = context->env;
			context->env = first;
		}
		else
		{
			// Uh oh. Out of memory.
			return - 1;
		}
	}
	
	return 0;
}

int console_unsetenv(pconsole context, const char* name)
{
	pconsole_env prev = NULL;
	pconsole_env cur = context->env;
	assert_if(context && name)
	{
		while (cur)
		{
			if (strcmp(cur->name, name) == 0)
			{
				goto delete;
			}
			
			prev = cur;
			cur = cur->next;
		}
		
		return -1;
	}
	
delete:
	if (prev)
	{
		prev->next = cur->next;
		vPortFree(cur);
	}
	else
	{
		vPortFree(context->env);
		context->env = cur;
	}
	
	return 0;
}

int console_getline(pconsole context, ConsoleFlags flags, char* line, size_t len)
{
	int rc = 0;
	int off = 0;
	assert_if(context)
	{
		memset(line, 0, len);
		while (off < len && (rc = console_getchar(context)) >= 0)
		{
			if (rc == '\b')
			{
				if (off > 0)
				{
					--off;	
				}
				else
				{
					continue;
				}
			}
		
			if (off < len)
			{
				char ch = rc;
				if (flags && ConsoleFlagsEchoOff)
				{
					if ((rc = console_putchar(context, ch)) < 0)
					{
						goto done;
					}	
				}
				
				if (ch == '\n')
				{
					line[off++] = 0;
					break;
				}
				else if (ch != '\b' && ch != '\r')
				{
					line[off++] = ch;	
				}
			}
		}
	}
	
done:
	if (rc < 0)
	{
		return rc;
	}
	
	return off;
}