/**
 _     _____ ____     __  __    _  _____ ____  _____  __
| |   | ____|  _ \   |  \/  |  / \|_   _|  _ \|_ _\ \/ /
| |   |  _| | | | |  | |\/| | / _ \ | | | |_) || | \  /
| |___| |___| |_| |  | |  | |/ ___ \| | |  _ < | | /  \
|_____|_____|____/___|_|  |_/_/   \_\_| |_| \_\___/_/\_\
                |_____|
****************************************************************************************************
*    @file           : main.cpp
*    @brief          : LED Matrix Panel Controller Firmware
****************************************************************************************************
*    @author     Volodymyr Noha
*
*    @description:
*    This is a main source file for the LED matrix panel controller firmware.
*    The device displays various text and graphics on an RGB LED matrix panel for lab MVC and RVC
*    camera video recording tests.
*
*    @section  HISTORY
*    v1.0  - First version
*
*    @section  LICENSE
*    None
****************************************************************************************************
*/

#include <Arduino.h>
#include <string.h>
#include <stdlib.h>

/* Include Adafruit GFX library */
#include "led_matrix_types.h"
#include "serial_logger.h"
#include "cmd.h"

#define NUMBER_OF_COMMANDS    6

#define MATRIX_WIDTH          64

#define CLK                   (uint8_t)11
#define OE                    (uint8_t)9
#define LAT                   (uint8_t)10

#define A                     A0
#define B                     A1
#define C                     A2
#define D                     A3

/* Create matrix panel object */
RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false, MATRIX_WIDTH);
Cmd *cmd;

/* Prototypes */

static
led_matrix_status_t matrix_print_text(text_params_t_st *params);

static
led_matrix_status_t print_screen_init_message(void);

static
led_matrix_status_t print_test_completed(void);

static
void run_countdown_tests(Cmd *thisCmd, char *command, bool printHelp);

static
void run_scrolling_text_test(Cmd *thisCmd, char *command, bool printHelp);

static
void run_fill_screen_test(Cmd *thisCmd, char *command, bool printHelp);

static
void run_vertical_line_test(Cmd *thisCmd, char *command, bool printHelp);

static
void run_horizontal_line_test(Cmd *thisCmd, char *command, bool printHelp);

static
void fill_screen(text_color_t_en color, uint32_t delay_ms);

static
void unrecognized_command(Cmd *thisCmd, char *command, bool printHelp);

static
void print_help(Cmd *thisCmd, char *command, bool printHelp);

/* Implementation */

/**
 * @brief Print text on the matrix panel display
 * @param params Pointer to text parameters structure
 */
static
led_matrix_status_t matrix_print_text(text_params_t_st *params) {
  if (NULL == params) {
    LOG_ERROR("Invalid parameters for matrix_print_text.");

    return LED_MATRIX_ERROR_INVALID_ARGUMENTS;
  }

  matrix.setTextSize(params->pixels_size);
  matrix.setTextWrap(false);
  matrix.setFont(params->f);
  matrix.setCursor(params->x, params->y);
  matrix.setTextColor(params->color);
  matrix.println(params->str);

  return LED_MATRIX_SUCCESS;
}

/**
 * @brief Print initial message on the LED matrix panel
 */
static
led_matrix_status_t print_screen_init_message(void) {
  text_params_t_st params = {0};
  led_matrix_status_t ret = LED_MATRIX_SUCCESS;

  params.x = 1;
  params.y = 5;
  params.str = (char *)"LED Matrix";
  params.f = NULL;
  params.color = COLOR_CYAN;
  params.pixels_size = SIZE_1_PIXEL;

  ret = matrix_print_text(&params);
  if (LED_MATRIX_SUCCESS != ret) {
    LOG_ERROR("Failed to print initial message on the LED matrix panel.");

    return ret;
  }

  params.x = 1;
  params.y = 15;
  params.str = (char *)"Inited";
  params.f = NULL;
  params.color = COLOR_CYAN;
  params.pixels_size = SIZE_1_PIXEL;

  ret = matrix_print_text(&params);
  if (LED_MATRIX_SUCCESS != ret) {
    LOG_ERROR("Failed to print initial message on the LED matrix panel.");

    return ret;
  }

  return ret;
}

/**
 * @brief Print test complete message on the LED matrix panel
 */
static
led_matrix_status_t print_test_completed(void) {
  text_params_t_st params = {0};
  led_matrix_status_t ret = LED_MATRIX_SUCCESS;

  /* Clear display after test */
  matrix.fillScreen(COLOR_BLACK);

  params.x = 1;
  params.y = 1;
  params.str = (char *)"Test";
  params.f = NULL;
  params.color = COLOR_GREEN;
  params.pixels_size = SIZE_1_PIXEL;

  ret = matrix_print_text(&params);
  if (LED_MATRIX_SUCCESS != ret) {
    LOG_ERROR("Failed to print 'Test' on the LED matrix panel.");

    return ret;
  }

  params.x = 1;
  params.y = 10;
  params.str = (char *)"Completed";
  params.f = NULL;
  params.color = COLOR_GREEN;
  params.pixels_size = SIZE_1_PIXEL;

  ret = matrix_print_text(&params);
  if (LED_MATRIX_SUCCESS != ret) {
    LOG_ERROR("Failed to print 'Completed' on the LED matrix panel.");

    return ret;
  }

  return ret;
}

/**
 * @brief Fill the LED matrix panel with a specific color for a duration
 * @param color Color to fill the screen with
 * @param delay_ms Duration to hold the color in milliseconds
 */
static
void fill_screen(text_color_t_en color, uint32_t delay_ms) {
  matrix.fillScreen(color);
  delay(delay_ms);
}

/**
 * @brief Handle unrecognized command and show help
 * @param Cmd pointer to command object
 * @param command Command string
 * @param printHelp Flag indicating whether to print help
 */
static
void print_help(Cmd *thisCmd, char *command, bool printHelp) {
	Serial.print("Available commands:\r\n\r\n");
  Serial.print("\thelp: \t\t\t\t\t\t\t Shows this help message\r\n");
  Serial.print("\trun_scrolling_text_test <delay_ms>: \t\t\t Runs a scrolling text test\r\n");
  Serial.print("\trun_countdown_test <countdown_seconds> <delay_ms>: \t Runs a countdown test with specified delay\r\n");
  Serial.print("\trun_fill_screen_test <delay_ms>: \t\t\t Fills the screen with each color\r\n");
  Serial.print("\trun_vertical_line_test: \t\t\t\t Runs a vertical line test\r\n");
  Serial.print("\trun_horizontal_line_test: \t\t\t\t Runs a horizontal line test\r\n");
  Serial.print("\r\n");

	return;
}

/**
 * @brief Handle unrecognized command
 * @param Cmd pointer to command object
 * @param command Command string
 * @param printHelp Flag indicating whether to print help
 */
static
void unrecognized_command(Cmd *thisCmd, char *command, bool printHelp) {

  if (NULL == thisCmd || NULL == command) {
    LOG_ERROR("Invalid arguments for unrecognized_command.");

    return;
  }

  Serial.print("Unrecognized command: ");
  Serial.println(command);
  Serial.print("Type 'help' to see available commands.\r\n");
}

/**
 * @brief Run timer countdown test on the LED matrix panel
 * @param Cmd pointer to command object
 * @param command Command string
 * @param printHelp Flag indicating whether to print help
 */
static
void run_countdown_tests(Cmd *thisCmd, char *command, bool printHelp) {
  led_matrix_status_t ret = LED_MATRIX_SUCCESS;
  text_params_t_st params = {0};
  char buffer[4];
  char *parsed = NULL;
  uint32_t seconds = 0, delay_ms = 0;

  if (NULL == thisCmd || NULL == command) {
    LOG_ERROR("Invalid arguments for run_countdown_test command.");

    return;
  }

  /* Parse the next available argument. */
	parsed = cmd->Parse();
	if (parsed == NULL) {
		LOG_ERROR("Invalid seconds");

		return;
	}
	/* Parse integer. */
	seconds = atoi(parsed);
  if (seconds == 0 || seconds > 999) {
    LOG_ERROR("Seconds must be greater than zero and less than 1000.");

    return;
  }

  /* Parse the next available argument. */
	parsed = cmd->Parse();
	if (parsed == NULL) {
		LOG_ERROR("Invalid delay_ms");

		return;
	}
	/* Parse integer. */
	delay_ms = atoi(parsed);
  if (delay_ms == 0) {
    LOG_ERROR("Delay_ms must be greater than zero.");

    return;
  }

  if (delay_ms < 100 || delay_ms > 10000) {
    LOG_ERROR("Delay_ms must be between 100 and 10000.");

    return;
  }

  params.x = 27;
  params.y = 11;
  params.f = NULL;
  params.color = COLOR_MAGENTA;
  params.pixels_size = SIZE_1_PIXEL;

  LOG_DEBUG("Running countdown test with seconds=%ld and delay_ms=%ld...", seconds, delay_ms);

  for (int i = seconds; i >= 0; i--) {
    /* Clear display */
    matrix.fillScreen(COLOR_BLACK);

    /* Prepare string */
    snprintf(buffer, sizeof(buffer), "%02d", i);

    /* Update string in parameters */
    params.str = buffer;

    /* Print text */
    ret = matrix_print_text(&params);
    if (LED_MATRIX_SUCCESS != ret) {
      LOG_ERROR("Failed to print countdown text on the LED matrix panel.");

      return;
    }

    /* Wait for 1 second */
    delay(delay_ms);
  }

  ret = print_test_completed();
  if (LED_MATRIX_SUCCESS != ret) {
    LOG_ERROR("Failed to print 'Test Completed' on the LED matrix panel.");
  }

  LOG_DEBUG("Running time test complete.");

  delay(2000);
  matrix.fillScreen(COLOR_BLACK);
}

/**
 * @brief Run vertical line test on the LED matrix panel
 * @param Cmd pointer to command object
 * @param command Command string
 * @param printHelp Flag indicating whether to print help
 */
static
void run_vertical_line_test(Cmd *thisCmd, char *command, bool printHelp) {
  led_matrix_status_t ret = LED_MATRIX_SUCCESS;

  if (NULL == thisCmd || NULL == command) {
    LOG_ERROR("Invalid arguments for run_vertical_line_test command.");

    return;
  }

  for(size_t i = 0; i < 6; i++) {
    for (int16_t x = 0; x < MATRIX_WIDTH; x++) {
      /* Clear display */
      matrix.fillScreen(COLOR_BLACK);

      /* Draw a vertical line at position x */
      matrix.drawFastVLine(x, 0, matrix.height(), COLOR_GREEN);

      /* Wait */
      delay(80);
    }
  }

  ret = print_test_completed();
  if (LED_MATRIX_SUCCESS != ret) {
    LOG_ERROR("Failed to print 'Test Completed' on the LED matrix panel.");
  }

  LOG_DEBUG("Running vertical line test complete.");

  delay(2000);
  matrix.fillScreen(COLOR_BLACK);
}

/**
 * @brief Run horizontal line test on the LED matrix panel
 * @param Cmd pointer to command object
 * @param command Command string
 * @param printHelp Flag indicating whether to print help
 */
static
void run_horizontal_line_test(Cmd *thisCmd, char *command, bool printHelp) {
  led_matrix_status_t ret = LED_MATRIX_SUCCESS;

  if (NULL == thisCmd || NULL == command) {
    LOG_ERROR("Invalid arguments for run_horizontal_line_test command.");

    return;
  }

  for (size_t i = 0; i < 6; i++) {
    for (int16_t y = 0; y < matrix.height(); y++) {
      /* Clear display */
      matrix.fillScreen(COLOR_BLACK);

      /* Draw a horizontal line at position y */
      matrix.drawFastHLine(0, y, MATRIX_WIDTH, COLOR_BLUE);

      /* Wait */
      delay(80);
    }
  }

  ret = print_test_completed();
  if (LED_MATRIX_SUCCESS != ret) {
    LOG_ERROR("Failed to print 'Test Completed' on the LED matrix panel.");
  }

  LOG_DEBUG("Running horizontal line test complete.");

  delay(2000);
  matrix.fillScreen(COLOR_BLACK);
}

/**
 * @brief Run scrolling text test on the LED matrix panel
 * @param Cmd pointer to command object
 * @param command Command string
 * @param printHelp Flag indicating whether to print help
 */
static
void run_scrolling_text_test(Cmd *thisCmd, char *command, bool printHelp) {
  led_matrix_status_t ret = LED_MATRIX_SUCCESS;
  text_params_t_st params = {0};
  const char *str = "This is a long text scrolling across the screen to test RVC and MVC camera recording.";
  size_t str_len = strlen(str);
  int16_t x_start = 64;
  int16_t x_end = -((int16_t)str_len * 6);

  char *parsed = NULL;
  uint32_t delay_ms = 0;

  if (NULL == thisCmd || NULL == command) {
    LOG_ERROR("Invalid arguments for run_scrolling_text_test command.");

    return;
  }

  /*Parse the next available argument. */
	parsed = cmd->Parse();
	if (parsed == NULL) {
		LOG_ERROR("Invalid delay_ms");

		return;
	}
	/* Parse integer. */
	delay_ms = atoi(parsed);
  if (delay_ms < 1 || delay_ms > 500) {
    LOG_ERROR("Delay_ms must be between 1 and 500.");

    return;
  }

  params.y = 11;
  params.f = NULL;
  params.color = COLOR_RED;
  params.pixels_size =SIZE_1_PIXEL;
  params.str = (char *)str;

  matrix.setTextWrap(false);

  LOG_DEBUG("Running running text test with delay_ms=%ld...", delay_ms);

  for (size_t i = 0; i < 3; i++) {
    for (int16_t x = x_start; x >= x_end; x--) {
      /* Clear display */
      matrix.fillScreen(COLOR_BLACK);

      params.x = x;

      /* Print text */
      ret = matrix_print_text(&params);
      if (LED_MATRIX_SUCCESS != ret) {
        LOG_ERROR("Failed to print scrolling text on the LED matrix panel.");

        return;
      }

      /* Wait */
      delay(delay_ms);
    }
  }

  ret = print_test_completed();
  if (LED_MATRIX_SUCCESS != ret) {
    LOG_ERROR("Failed to print 'Test Completed' on the LED matrix panel.");
  }

  LOG_DEBUG("Running text test complete.");

  delay(2000);
  matrix.fillScreen(COLOR_BLACK);
}

/**
 * @brief Run fill screen color test on the LED matrix panel
 * @param Cmd pointer to command object
 * @param command Command string
 * @param printHelp Flag indicating whether to print help
 */
static
void run_fill_screen_test(Cmd *thisCmd, char *command, bool printHelp) {
  led_matrix_status_t ret = LED_MATRIX_SUCCESS;
  char *parsed = NULL;
  uint32_t delay_ms = 0;

  if (NULL == thisCmd || NULL == command) {
    LOG_ERROR("Invalid arguments for run_fill_screen_test command.");

    return;
  }

  /* Parse the next available argument. */
	parsed = cmd->Parse();
	if (parsed == NULL) {
		LOG_ERROR("Invalid delay_ms");

		return;
	}
	/* Parse integer. */
	delay_ms = atoi(parsed);
  if (delay_ms < 100 || delay_ms > 10000) {
    LOG_ERROR("Delay_ms must be between 100 and 10000.");

    return;
  }

  LOG_DEBUG("Running fill screen color test with delay_ms=%ld...", delay_ms);

  for (size_t i = 0; i < 3; i++)
  {
    matrix.fillScreen(COLOR_BLACK);

    LOG_DEBUG("Filling screen with red color ");
    fill_screen((text_color_t_en)COLOR_RED, delay_ms);

    LOG_DEBUG("Filling screen with green color ");
    fill_screen((text_color_t_en)COLOR_GREEN, delay_ms);

    LOG_DEBUG("Filling screen with blue color ");
    fill_screen((text_color_t_en)COLOR_BLUE, delay_ms);

    LOG_DEBUG("Filling screen with yellow color ");
    fill_screen((text_color_t_en)COLOR_YELLOW, delay_ms);

    LOG_DEBUG("Filling screen with cyan color ");
    fill_screen((text_color_t_en)COLOR_CYAN, delay_ms);

    LOG_DEBUG("Filling screen with magenta color ");
    fill_screen((text_color_t_en)COLOR_MAGENTA, delay_ms);
  }

  ret = print_test_completed();
  if (LED_MATRIX_SUCCESS != ret) {
    LOG_ERROR("Failed to print 'Test Completed' on the LED matrix panel.");
  }

  LOG_DEBUG("Fill screen color test complete.");

  delay(2000);
  matrix.fillScreen(COLOR_BLACK);
}

/**
 * @brief Arduino setup function
 */
void setup() {
  led_matrix_status_t ret = LED_MATRIX_SUCCESS;

  setup_serial_logger();

  LOG_INFO("Serial logger initialized.");

  /* Initialize matrix controller */
  LOG_INFO("Initializing LED matrix controller...");
  matrix.begin();
  LOG_INFO("LED matrix controller initialized.");

	/* Initialize the command line with 2 commands. */
	cmd = new Cmd(NUMBER_OF_COMMANDS, unrecognized_command);

	/* Add commands. */
  cmd->AddCmd(PSTR("help"), print_help);
	cmd->AddCmd(PSTR("run_scrolling_text_test"), run_scrolling_text_test);
	cmd->AddCmd(PSTR("run_countdown_test"), run_countdown_tests);
  cmd->AddCmd(PSTR("run_fill_screen_test"), run_fill_screen_test);
  cmd->AddCmd(PSTR("run_vertical_line_test"), run_vertical_line_test);
  cmd->AddCmd(PSTR("run_horizontal_line_test"), run_horizontal_line_test);

	/* Print a line indicator to inform the user the cli is ready. */
  cmd->SetLineIndicator("> ");
	Serial.print(cmd->GetLineIndicator());

  /* Print screen init message */
  ret = print_screen_init_message();
  if (LED_MATRIX_SUCCESS != ret) {
    LOG_ERROR("Failed to print screen init message on the LED matrix panel.");
  }
}

/**
 * @brief Arduino loop function
 */
void loop() {
  cmd->Loop();
}

