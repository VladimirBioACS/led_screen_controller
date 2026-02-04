#ifndef _CMD_H_
#define _CMD_H_

#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

class Cmd;

typedef void (*CmdFunction)(Cmd *thisCmd, char *command, bool printHelp);

class Cmd {
 protected:
	const char **m_commands = NULL;
	CmdFunction *m_functions = NULL;

	CmdFunction m_defaultFunction;

	size_t m_size = 0;
	size_t m_nextCmd = 0;

	bool m_echo = true;
	bool m_processing = false;
	const char *m_separator = " ";
	const char *m_line_indicator = "$ ";
	size_t m_buffer_size = 50;

	char *m_buffer = NULL;
	char *m_bufferTok = NULL;
	size_t m_buffer_read = 0;
	uint8_t m_buffer_reading_esc = 0;
	size_t m_buffer_cursor = 0;

	void PrintHelp();
	void ParseBuffer();
	void StartNewBuffer();

 public:
	Cmd(size_t size, CmdFunction defaultCallback);

	size_t GetSize();
	bool AddCmd(const char *cmd, CmdFunction function);
	const char **GetCmds();

	bool GetEcho();
	void SetEcho(bool echo);

	const char *GetSeparator();
	void SetSeparator(const char *separator);

	const char *GetLineIndicator();
	void SetLineIndicator(const char *line_indicator);

	size_t GetBufferSize();
	void SetBufferSize(size_t bufferSize);

	const char *GetBuffer();
	void PrintBuffer();
	char *Parse();

	void SendESC(const char *code);

	void Loop();
};

#endif  // _CMD_H_