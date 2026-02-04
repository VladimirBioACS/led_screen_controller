#include "cmd.h"

// Initiator.
// Size is the number of commands, default callback is called if a command is
// not found.
Cmd::Cmd(size_t size, CmdFunction defaultCallback) {
	m_size = size;
	m_commands = (const char **)malloc(sizeof(const char *) * m_size);
	m_functions = (CmdFunction *)malloc(sizeof(CmdFunction) * m_size);
	m_defaultFunction = defaultCallback;
}

// Return the set size of the command array.
size_t Cmd::GetSize() { return m_size; }

// Add a command to the function list. If list is too full, false is returned.
bool Cmd::AddCmd(const char *cmd, CmdFunction function) {
	if (m_nextCmd >= m_size) {
		return false;
	}

	m_commands[m_nextCmd] = cmd;
	m_functions[m_nextCmd] = function;

	m_nextCmd++;
	return true;
}

// Get array of all commands.
const char **Cmd::GetCmds() { return m_commands; }

// Rather or not we echo back to serial characters received.
bool Cmd::GetEcho() { return m_echo; }
void Cmd::SetEcho(bool echo) { m_echo = echo; }

// The separator for command parsing.
const char *Cmd::GetSeparator() { return m_separator; }
void Cmd::SetSeparator(const char *separator) { m_separator = separator; }

// Indicates the command line.
const char *Cmd::GetLineIndicator() { return m_line_indicator; }
void Cmd::SetLineIndicator(const char *line_indicator) {
	m_line_indicator = line_indicator;
}

// Buffer size configuration.
size_t Cmd::GetBufferSize() { return m_buffer_size; }
void Cmd::SetBufferSize(size_t bufferSize) { m_buffer_size = bufferSize; }

// Return current buffer.
const char *Cmd::GetBuffer() { return m_buffer; }

// Resets the buffer and prints
void Cmd::StartNewBuffer() {
	free(m_buffer);
	m_buffer = NULL;
	Serial.print(m_line_indicator);
}

// Print the current buffer.
void Cmd::PrintBuffer() {
	// If we're processing a command, do not print.
	if (m_processing) {
		return;
	}
	// Set the last character in the buffer to null to terminate.
	m_buffer[m_buffer_read] = '\0';
	// Print indicator and buffer.
	Serial.print(m_line_indicator);
	Serial.print(m_buffer);
	// Move cursor to correct location.
	for (unsigned int i = m_buffer_cursor; i < m_buffer_read; i++) {
		Serial.write('\x08');
	}
}

// Parse next token.
char *Cmd::Parse() { return strtok(NULL, m_separator); }

// Print help for the current command.
void Cmd::PrintHelp() {
	// Copy buffer to token buffer.
	m_bufferTok = (char *)malloc(m_buffer_size);
	strcpy(m_bufferTok, m_buffer);

	// Tokenize buffer based on separator and get first token being the command.
	char *cmd = strtok(m_bufferTok, m_separator);

	// Printing help means the command line is currently the line we're on.
	Serial.println();

	// To prevent buffer printing during a command execution.
	m_processing = true;
	// Look for the command that matches.
	bool foundCmd = false;
	// Only scan for command if specified.
	if (cmd != NULL) {
		for (unsigned int i = 0; i < m_size && m_commands[i] != NULL; i++) {
			if (strcasecmp_P(cmd, m_commands[i]) == 0) {
				// If command matches, call its function and tell it we're asking for
				// help.
				m_functions[i](this, cmd, true);
				foundCmd = true;
				break;
			}
		}
	}

	// If command wasn't found, call the default callback and tell it we're asking
	// for help.
	if (!foundCmd) {
		Serial.println("Calling default function");
		m_defaultFunction(this, cmd, true);
	}
	m_processing = false;

	// Print the buffer now that help was provided.
	Serial.println();
	PrintBuffer();

	// Free memory used by the token buffer.
	free(m_bufferTok);
	m_bufferTok = NULL;
}

// Parse buffer for command.
void Cmd::ParseBuffer() {
	// Copy buffer to token buffer.
	m_bufferTok = (char *)malloc(m_buffer_size);
	strcpy(m_bufferTok, m_buffer);

	// Tokenize buffer based on separator and get first token being the command.
	char *cmd = strtok(m_bufferTok, m_separator);

	// To prevent buffer printing during a command execution.
	m_processing = true;
	// Look for the command that matches.
	bool foundCmd = false;
	// Only scan for command if specified.
	if (cmd != NULL) {
		for (unsigned int i = 0; i < m_size && m_commands[i] != NULL; i++) {
			if (strcasecmp_P(cmd, m_commands[i]) == 0) {
				// If command matches, call its function.
				m_functions[i](this, cmd, false);
				foundCmd = true;
				break;
			}
		}
	}

	// If command wasn't found, call the default callback.
	if (!foundCmd) {
		m_defaultFunction(this, cmd, false);
	}
	m_processing = false;

	// Free memory used by the token buffer.
	free(m_bufferTok);
	m_bufferTok = NULL;
}

// Main command loop, call in the main loop of your program.
void Cmd::Loop() {
	// If no serial data is available, we do not have anything to process.
	if (!Serial.available()) {
		return;
	}

	// If buffer is not allocated, let's reset.
	if (m_buffer == NULL) {
		m_buffer = (char *)malloc(m_buffer_size);
		m_buffer_read = 0;
		m_buffer_cursor = 0;
	}

	// Start read and read all available data in serial RX buffer.
	bool receivedEndLine = false;
	size_t availableData = Serial.available();
	for (size_t i = 1; i <= availableData; i++) {
		char byteRead = Serial.read();

		// If we're currently reading an escape character, verify state.
		if (m_buffer_reading_esc == 1) {
			// If we get the bracket, we need to move on to the next step in reading
			// an escape character. Otherwise, we are not reading an escape character.
			if (byteRead == '[') {
				m_buffer_reading_esc = 2;
				continue;
			} else {
				m_buffer_reading_esc = 0;
			}
		} else if (m_buffer_reading_esc == 2) {
			// Process escape character read.
			switch (byteRead) {
				case 'D':  // Move cursor left
					m_buffer_reading_esc = 0;
					if (m_buffer_cursor <= 0) {
						continue;
					}
					m_buffer_cursor--;
					Serial.print("\x1b[D");
					break;
				case 'C':  // Move cursor right
					m_buffer_reading_esc = 0;
					if (m_buffer_cursor >= m_buffer_read) {
						continue;
					}
					m_buffer_cursor++;
					Serial.print("\x1b[C");
					break;

				default:
					m_buffer_reading_esc = 0;
					break;
			}
			continue;
		}

		// Check if ascii byte.
		bool is_ascii = byteRead >= 32 && byteRead <= 126;

		// If we're to echo and its an ascii byte, send the byte we read back to the
		// serial console.
		if (m_echo && is_ascii) {
			Serial.write(byteRead);
		}

		// If escape character received, move into escape reading mode.
		if (byteRead == '\x1b') {
			m_buffer_reading_esc = 1;
			continue;
		}

		// If backspace or delete key.
		if (byteRead == 8 || byteRead == 127) {
			// Only perform character delete if buffer exists and cursor isn't at
			// start.
			if (m_buffer_read != 0 && m_buffer_cursor != 0) {
				// If cursor isn't at start, we need to re-print the line minus the
				// character deleted.
				if (m_buffer_cursor != m_buffer_read) {
					// Create new buffer for re-writing current buffer.
					char *buf = (char *)malloc(m_buffer_size);
					// Copy current buffer.
					strcpy(buf, m_buffer);
					// Clear the line from the curosr.
					Serial.print("\x08\x1b[1P");
					// Print and re-write the buffer from the cursor location minus
					// character deleted.
					for (unsigned int i = m_buffer_cursor; i < m_buffer_read; i++) {
						m_buffer[i - 1] = buf[i];
						Serial.write(buf[i]);
					}
					// Now that the buffer has been re-written, we can free the buffer.
					free(buf);
					// Move the cursor back to where it should be.
					for (unsigned int i = m_buffer_cursor; i < m_buffer_read; i++) {
						Serial.write('\x08');
					}
				} else {
					// As we're not deleting from cursor location, we can just clear the
					// last character via this escape.
					Serial.print("\x08\x1b[K");
				}
				// Wipe the last byte in the buffer in both cases.
				m_buffer_read--;
				m_buffer_cursor--;
				m_buffer[m_buffer_read] = '\0';
			}
		}

		// If begining of line requested.
		if (byteRead == 1) {
			// Move cursor to beginning of line.
			while (m_buffer_cursor != 0) {
				m_buffer_cursor--;
				Serial.print("\x1b[D");
			}
		}

		// If end of line requested.
		if (byteRead == 5) {
			// Move cursor to end of line.
			while (m_buffer_cursor < m_buffer_read) {
				m_buffer_cursor++;
				Serial.print("\x1b[C");
			}
		}

		// If cancel or exit received.
		if (byteRead == 3 || byteRead == 4) {
			Serial.println();
			// Clear buffer, start new line,
			StartNewBuffer();
			continue;
		}

		// Clear screen.
		if (byteRead == 12) {
			m_buffer[m_buffer_read] = '\0';
			Serial.print("\x1b[H\x1b[J");
			PrintBuffer();
		}

		// If end of line.
		if (byteRead == '\r') {
			receivedEndLine = true;
			break;
		}

		// If help requested, print it.
		if (byteRead == '?') {
			m_buffer[m_buffer_read] = '\0';
			PrintHelp();
			continue;
		}

		// If not ascii, we only allow ascii on the cli.
		if (!is_ascii) {
			continue;
		}

		// If cursor is not at end, we need to write new byte where cursor is.
		if (m_buffer_cursor != m_buffer_read) {
			// Copy current buffer for re-write.
			char *buf = (char *)malloc(m_buffer_size);
			strcpy(buf, m_buffer);
			// Set current cursor location byte to newly read byte.
			m_buffer[m_buffer_cursor] = byteRead;
			// From cursor location, re-write the buffer with old buffer data and
			// print to the console.
			for (unsigned int i = m_buffer_cursor; i < m_buffer_read; i++) {
				m_buffer[i + 1] = buf[i];
				Serial.write(buf[i]);
			}
			// We're done with the buffer copy.
			free(buf);
			// Move cursor back to where it was.
			for (unsigned int i = m_buffer_cursor; i < m_buffer_read; i++) {
				Serial.write('\x08');
			}
		} else {
			// Otherwise new byte can go to end of buffer.
			m_buffer[m_buffer_read] = byteRead;
		}
		// We read a new byte, increment the cursor location and read index.
		m_buffer_read++;
		m_buffer_cursor++;

		// If we're going to overflow next read, we need to stop that.
		if (m_buffer_read >= (m_buffer_size - 1)) {
			Serial.println("Data too large.");

			// Clear the buffer, and start new line.
			StartNewBuffer();

			// Clear the serial buffer.
			while (Serial.available()) {
				Serial.read();
			}
			break;
		}
	}

	// If we received end of line in read, we need to parse the buffer.
	if (receivedEndLine) {
		// Print new line and null terminate the buffer.
		Serial.println();
		m_buffer[m_buffer_read] = '\0';

		// Parse the buffer.
		ParseBuffer();

		// Start new buffer.
		StartNewBuffer();
	}
}