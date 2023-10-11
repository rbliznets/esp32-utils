/*!
	\file
	\brief Класс для вывода отладочной информации в файл.
	\authors Близнец Р.А. (r.bliznets@gmail.com)
	\version 1.0.0.0
	\date 05.09.2023

	Один объект на приложение.
	Необходим чтобы не блокировать отлаживаемую задачу.
*/

#if !defined CSPIFFSTRACETASK_H
#define CSPIFFSTRACETASK_H

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "CTraceTask.h"
#include <cstdio>

/// Класс задачи вывода отладочной информации в файл.
class CSpiffsTraceTask : public CTraceTask
{
private:
	FILE *mLogFile = nullptr; ///< Файл записи логов.

protected:
	/// Вывести интервал времени с предыдущего сообщения
	/*!
	  \param[in] time Сообщение об ошибке.
	  \param[in] n количество для усреднения.
	*/
	void printHeader(uint64_t time, uint32_t n = 1) override;

	/// Функция задачи.
	virtual void run() override;

	/// Вывести сообщение.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE_STRING или MSG_TRACE_STRING_REBOOT.
	*/
	void printString(char *data) override;
	/// Вывести сообщение об интервале времени.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_STOP_TIME.
	*/
	void printStop(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE_UINT8.
	*/
	void printData8h(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE2_UINT8.
	*/
	void printData8h_2(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE_UINT16.
	*/
	void printData16h(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE2_UINT16.
	*/
	void printData16h_2(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE_UINT32.
	*/
	void printData32h(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE2_UINT32.
	*/
	void printData32h_2(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE_INT8.
	*/
	void printData8(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE2_INT8.
	*/
	void printData8_2(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE_INT16.
	*/
	void printData16(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE2_INT16.
	*/
	void printData16_2(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE_INT32.
	*/
	void printData32(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE2_INT32.
	*/
	void printData32_2(char *data) override;

public:
	/// Единственный экземпляр класса.
	/*!
	  \return Указатель на CSpiffsTraceTask
	*/
	static CSpiffsTraceTask *Instance()
	{
		static CSpiffsTraceTask theSingleInstance;
		return &theSingleInstance;
	}
	/// Деструктор.
	virtual ~CSpiffsTraceTask(){};

	/// Начальная инициализация.
	/*!
	  \param[in] queueLength Максимальная длина очереди сообщений.
	  \param[in] coreID Ядро CPU (0,1).
	*/
	void init(UBaseType_t queueLength = 30, BaseType_t coreID = 1) override
	{
		CBaseTask::init("log", 4096, 1, queueLength, coreID);
	};
};

#endif // CSPIFFSTRACETASK_H
