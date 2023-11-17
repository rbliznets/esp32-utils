/*!
	\file
	\brief Класс консоли.
	\authors Близнец Р.А. (r.bliznets@gmail.com)
	\version 1.0.1.0
	\date 03.02.2023

	Один объект на приложение.
*/

#include "CConsole.h"
#include <cstring>
#include "esp_system.h"
#include "esp_log.h"
#include <cstdio>
#include <dirent.h>
#include "esp_spiffs.h"

static const char *TAG = "console";

enum class CAT_MODE
{
	Text,
	Hex8,
	Hex16,
	Hex32,
	Dec8,
	Dec16,
	Dec32
};

CConsole::~CConsole()
{
	stop();
}

void CConsole::stop()
{
	mRepl->del(mRepl);
	vTaskDelay(pdMS_TO_TICKS(10));
	esp_console_deinit();
}

void CConsole::start(onJsonCmdEvent *func, onExitCmdEvent *func2)
{
	// esp_console_config_t con_config = ESP_CONSOLE_CONFIG_DEFAULT();
	// con_config.max_cmdline_length = CONFIG_CONSOLE_MAX_CMDLINE_LENGTH;
	// esp_console_init(&con_config);

	esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
	repl_config.prompt = CONFIG_CONSOLE_PROMPT;
	repl_config.max_cmdline_length = CONFIG_CONSOLE_MAX_CMDLINE_LENGTH;
	repl_config.task_priority = 1;
	// repl_config.task_stack_size = 8192;

	esp_console_register_help_command();

	if (func != nullptr)
	{
		const esp_console_cmd_t json_cmd = {
			.command = "json",
			.help = "Send json command",
			.hint = "{Format json without spaces}",
			.func = &do_json_cmd,
			.argtable = nullptr};
		ESP_ERROR_CHECK(esp_console_cmd_register(&json_cmd));
	}

#ifdef CONFIG_UTILS_SPIFFS_INIT
	const esp_console_cmd_t ls_cmd = {
		.command = "ls",
		.help = "List files",
		.hint = nullptr,
		.func = &do_ls_cmd,
		.argtable = nullptr};
	ESP_ERROR_CHECK(esp_console_cmd_register(&ls_cmd));
	const esp_console_cmd_t cat_cmd = {
		.command = "cat",
		.help = "print file (text, decimal, hex)",
		.hint = "[-t|-d8|-d16|-d32|-h8|-h16|-h32] <filename>",
		.func = &do_cat_cmd,
		.argtable = nullptr};
	ESP_ERROR_CHECK(esp_console_cmd_register(&cat_cmd));
	const esp_console_cmd_t rm_cmd = {
		.command = "rm",
		.help = "delete file",
		.hint = "<filename>",
		.func = &do_rm_cmd,
		.argtable = nullptr};
	ESP_ERROR_CHECK(esp_console_cmd_register(&rm_cmd));
	const esp_console_cmd_t frm_cmd = {
		.command = "format",
		.help = "format spiffs",
		.hint = nullptr,
		.func = &do_frm_cmd,
		.argtable = nullptr};
	ESP_ERROR_CHECK(esp_console_cmd_register(&frm_cmd));
#endif // CONFIG_UTILS_SPIFFS_INIT

	if (func2 != nullptr)
	{
		const esp_console_cmd_t exit_cmd = {
			.command = "e",
			.help = "Close console",
			.hint = nullptr,
			.func = &do_exit_cmd,
			.argtable = nullptr};
		ESP_ERROR_CHECK(esp_console_cmd_register(&exit_cmd));
	}

#if CONFIG_ESP_CONSOLE_UART
	esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_console_new_repl_uart(&uart_config, &repl_config, &mRepl));
#elif CONFIG_ESP_CONSOLE_USB_CDC
	esp_console_dev_usb_cdc_config_t cdc_config = ESP_CONSOLE_DEV_CDC_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_console_new_repl_usb_cdc(&cdc_config, &repl_config, &mRepl));
#elif CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG
	esp_console_dev_usb_serial_jtag_config_t usbjtag_config = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_console_new_repl_usb_serial_jtag(&usbjtag_config, &repl_config, &mRepl));
#endif

	onCmd = func;
	onExit = func2;
	// vTaskDelay(pdMS_TO_TICKS(150));
	ESP_ERROR_CHECK(esp_console_start_repl(mRepl));
}

#ifdef CONFIG_UTILS_SPIFFS_INIT
int CConsole::do_frm_cmd(int argc, char **argv)
{
	if (argc != 1)
	{
		return ESP_ERR_INVALID_ARG;
	}
	else
	{
		if (esp_spiffs_format(nullptr) != ESP_OK)
		{
			return ESP_FAIL;
		}
		return 0;
	}
}

int CConsole::do_ls_cmd(int argc, char **argv)
{
	if (argc != 1)
	{
		return ESP_ERR_INVALID_ARG;
	}
	else
	{
		struct dirent *entry;
		DIR *dp;
		dp = opendir("/spiffs");
		if (dp == nullptr)
		{
			ESP_LOGE(TAG, "Failed to open dir /spiffs");
			return ESP_ERR_NOT_SUPPORTED;
		}
		std::printf("\033[43m\t name \t\t size \033[00m\n");
		std::string str = "/spiffs/";
		while ((entry = readdir(dp)))
		{
			FILE *f = std::fopen((str + entry->d_name).c_str(), "r");
			int32_t sz = -1;
			if (f != nullptr)
			{
				std::fseek(f, 0, SEEK_END);
				sz = std::ftell(f);
				std::fclose(f);
			}
			std::printf("\t%s\t\t%ld\n", entry->d_name, sz);
		}
		closedir(dp);
		return 0;
	}
}

int CConsole::do_cat_cmd(int argc, char **argv)
{
	if ((argc != 2) && (argc != 3))
	{
		return ESP_ERR_INVALID_ARG;
	}

	std::string str = "/spiffs/";
	CAT_MODE mode = CAT_MODE::Text;
	if (argc == 3)
	{
		if (std::strcmp("-t", argv[1]) == 0)
		{
			mode = CAT_MODE::Text;
		}
		else if (std::strcmp("-h8", argv[1]) == 0)
		{
			mode = CAT_MODE::Hex8;
		}
		else if (std::strcmp("-h16", argv[1]) == 0)
		{
			mode = CAT_MODE::Hex16;
		}
		else if (std::strcmp("-h32", argv[1]) == 0)
		{
			mode = CAT_MODE::Hex32;
		}
		else if (std::strcmp("-d8", argv[1]) == 0)
		{
			mode = CAT_MODE::Dec8;
		}
		else if (std::strcmp("-d16", argv[1]) == 0)
		{
			mode = CAT_MODE::Dec16;
		}
		else if (std::strcmp("-d32", argv[1]) == 0)
		{
			mode = CAT_MODE::Dec32;
		}
		else
		{
			return ESP_ERR_INVALID_ARG;
		}
		str += argv[2];
	}
	else
	{
		str += argv[1];
	}

	FILE *f = std::fopen(str.c_str(), "r");
	if (f != nullptr)
	{
		size_t size;
		char buf8[65];
		uint16_t *h16 = (uint16_t *)buf8;
		uint32_t *h32 = (uint32_t *)buf8;
		int8_t *d8 = (int8_t *)buf8;
		int16_t *d16 = (int16_t *)buf8;
		int32_t *d32 = (int32_t *)buf8;
		uint32_t sz = 0;
		while ((size = std::fread(buf8, 1, 64, f)) > 0)
		{
			switch (mode)
			{
			case CAT_MODE::Hex8:
				for (size_t i = 0; i < size; i++)
				{
					std::printf("0x%2X,", buf8[i]);
				}
				sz += size;
				break;
			case CAT_MODE::Hex16:
				for (size_t i = 0; i < size / 2; i++)
				{
					std::printf("0x%4X,", h16[i]);
				}
				sz += size / 2;
				break;
			case CAT_MODE::Hex32:
				for (size_t i = 0; i < size / 4; i++)
				{
					std::printf("0x%8lX,", h32[i]);
				}
				sz += size / 4;
				break;
			case CAT_MODE::Dec8:
				for (size_t i = 0; i < size; i++)
				{
					std::printf("%d,", d8[i]);
				}
				sz += size;
				break;
			case CAT_MODE::Dec16:
				for (size_t i = 0; i < size / 2; i++)
				{
					std::printf("%d,", d16[i]);
				}
				sz += size / 2;
				break;
			case CAT_MODE::Dec32:
				for (size_t i = 0; i < size / 4; i++)
				{
					std::printf("%ld,", d32[i]);
				}
				sz += size / 4;
				break;
			default:
				buf8[size] = 0;
				std::printf("%s", buf8);
				sz += size;
				break;
			}
		}
		std::fclose(f);
		std::printf("\n\nsize:%ld\n", sz);
		return 0;
	}
	ESP_LOGW(TAG, "Failed to open %s", str.c_str());
	return ESP_ERR_NOT_FOUND;
}

int CConsole::do_rm_cmd(int argc, char **argv)
{
	if (argc != 2)
	{
		return ESP_ERR_INVALID_ARG;
	}
	else
	{
		std::string str = "/spiffs/";
		str += argv[1];
		if (std::remove(str.c_str()) == 0)
		{
			return 0;
		}
		ESP_LOGW(TAG, "Failed to remove %s", str.c_str());
		return ESP_ERR_NOT_FOUND;
	}
}

#endif // CONFIG_UTILS_SPIFFS_INIT

int CConsole::do_exit_cmd(int argc, char **argv)
{
	if (argc != 1)
	{
		return ESP_ERR_INVALID_ARG;
	}
	else
	{
		CConsole::Instance()->stop();
		if (CConsole::Instance()->onExit != nullptr)
		{
			CConsole::Instance()->onExit();
		}
		return 0;
	}
}

int CConsole::do_json_cmd(int argc, char **argv)
{
	if (argc != 2)
	{
		return ESP_ERR_INVALID_ARG;
	}
	else
	{
		return CConsole::Instance()->onJsonCmd(argv[1]);
	}
}

int CConsole::onJsonCmd(const char *json)
{
	int t1 = mJson.parse(json);
	if (t1 != 1)
	{
		return ESP_FAIL;
	}

	if (onCmd != nullptr)
	{
		onCmd(&mJson);
	}

	return 0;
}

#ifdef CONFIG_CONSOLE_EXIT_TOUSB
#include "CUsbUart.h"

static void onConnect(bool con)
{
	if (con)
	{
		ESP_LOGI(TAG, "USB console");
	}
}

void CConsole::exitConsole()
{
	ESP_LOGI(TAG, "close console");
	vTaskDelay(pdMS_TO_TICKS(500));

	CUsbUart::Instance()->start(CConsole::Instance()->onCmd, onConnect);
}
#endif // CONFIG_TINYUSB_CDC_ENABLED