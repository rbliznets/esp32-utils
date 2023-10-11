/*!
	\file
	\brief Класс консоли.
	\authors Близнец Р.А. (r.bliznets@gmail.com)
	\version 1.0.1.0

	Один объект на приложение.
*/

#if !defined CCONSOLE_H
#define CCONSOLE_H

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_console.h"
#include "CJsonParser.h"

/// Функция события получения команды.
/*!
	\param[in] parser парсер json.
*/
typedef void onJsonCmdEvent(CJsonParser *parser);
/// Функция события выхода из консоли.
typedef void onExitCmdEvent();

/// Класс консоли.
class CConsole
{
protected:
	esp_console_repl_t *mRepl = nullptr; ///< Type defined for console REPL..

	/// Callback функция на команду json.
	/*!
	  \param[in] argc number of arguments.
	  \param[in] argv array with argc entries, each pointing to a zero-terminated string argument.
	  \return 0 в случае успеха, иначе ошибка
	*/
	static int do_json_cmd(int argc, char **argv);
	/// Callback функция на команду exit.
	/*!
	  \param[in] argc number of arguments.
	  \param[in] argv array with argc entries, each pointing to a zero-terminated string argument.
	  \return 0 в случае успеха, иначе ошибка
	*/
	static int do_exit_cmd(int argc, char **argv);

#ifdef CONFIG_CONSOLE_EXIT_TOUSB
	/// Событие на выход из консоли
	static void exitConsole();
#endif

#ifdef CONFIG_UTILS_SPIFFS_INIT
	/// Callback функция на команду ls.
	/*!
	  \param[in] argc number of arguments.
	  \param[in] argv array with argc entries, each pointing to a zero-terminated string argument.
	  \return 0 в случае успеха, иначе ошибка
	*/
	static int do_ls_cmd(int argc, char **argv);
	/// Callback функция на команду cat.
	/*!
	  \param[in] argc number of arguments.
	  \param[in] argv array with argc entries, each pointing to a zero-terminated string argument.
	  \return 0 в случае успеха, иначе ошибка
	*/
	static int do_cat_cmd(int argc, char **argv);
	/// Callback функция на команду rm.
	/*!
	  \param[in] argc number of arguments.
	  \param[in] argv array with argc entries, each pointing to a zero-terminated string argument.
	  \return 0 в случае успеха, иначе ошибка
	*/
	static int do_rm_cmd(int argc, char **argv);
	/// Callback функция на команду format.
	/*!
	  \param[in] argc number of arguments.
	  \param[in] argv array with argc entries, each pointing to a zero-terminated string argument.
	  \return 0 в случае успеха, иначе ошибка
	*/
	static int do_frm_cmd(int argc, char **argv);
#endif // CONFIG_UTILS_SPIFFS_INIT

	CJsonParser mJson;				 ///< Данные парсера.
	onJsonCmdEvent *onCmd = nullptr; ///< Обработка события команды json.
	/// Задекорированная сallback функция на команду json.
	/*!
	  \param[in] json аргумент команды.
	  \return 0 в случае успеха, иначе ошибка
	*/
	int onJsonCmd(const char *json);

	onExitCmdEvent *onExit = nullptr; ///< Обработка события команды exit.

public:
	/// Единственный экземпляр класса.
	/*!
	  \return Указатель на CConsole
	*/
	static CConsole *Instance()
	{
		static CConsole theSingleInstance;
		return &theSingleInstance;
	}
	/// Деструктор.
	virtual ~CConsole();

#ifdef CONFIG_CONSOLE_EXIT_TOUSB
	/// Запуск консоли.
	/*!
	  \param[in] func Обработчик json команды.
	  \param[in] func2 Обработчик exit команды.
	*/
	void start(onJsonCmdEvent *func = nullptr, onExitCmdEvent *func2 = exitConsole);
#else
	/// Запуск консоли.
	/*!
	  \param[in] func Обработчик json команды.
	  \param[in] func2 Обработчик exit команды.
	*/
	void start(onJsonCmdEvent *func = nullptr, onExitCmdEvent *func2 = nullptr);
#endif // CONFIG_CONSOLE_EXIT_TOUSB
	/// Остановка консоли.
	void stop();

	/// Запуск команды.
	/*!
	  \param[in] cmdline Строка команды.
	  \return ESP_OK в случае успеха.
	*/
	inline esp_err_t run(const char *cmdline)
	{
		int x;
		return esp_console_run(cmdline, &x);
	};
};

#endif // CCONSOLE_H
