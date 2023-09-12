/*!
	\file
	\brief Класс обертка для tinyUSB СDС.
	\authors Близнец Р.А.
	\version 1.0.0.0

	Один объект на приложение.
*/

#if !defined CUSBUART_H
#define CUSBUART_H

#include "sdkconfig.h"
#ifdef CONFIG_TINYUSB_CDC_ENABLED

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "tinyusb.h"
#include "tusb_console.h"
#include "tusb_cdc_acm.h"

#include "CBaseTask.h"
#include "CJsonParser.h"

#define CONSOLE_MAX_STRING (1024)

/// Функция события получения команды.
/*!
	\param[in] parser парсер json.
*/
typedef void onJsonCmdEvent(CJsonParser* parser);
/// Функция события на установку соединения.
/*!
	\param[in] con true - подключение, false - отключение.
*/
typedef void onConectEvent(bool con);


/// Класс обертка для tinyUSB СDС.
class CUsbUart : public CBaseTask
{
protected:
	static bool mConsole0;	///< Флаг перенаправления print в консоль.
	/// функция обработки данных из CDC.
	/*!
	  \param[in] itf номер CDC.
	  \param[in] event параметры callback функции.
	*/
	static void cdc_rx_callback(int itf, cdcacm_event_t *event);
	/// функция обработки изменения состояния CDC.
	/*!
	  \param[in] itf номер CDC.
	  \param[in] event параметры callback функции.
	*/
	static void cdc_line_state_changed_callback(int itf, cdcacm_event_t *event);

	/// функция обработки данных из CDC для консоли.
	/*!
	  \param[in] itf номер CDC.
	*/
	void console_rx(tinyusb_cdcacm_itf_t itf);


	/// Функция задачи.
	virtual void run() override;

	uint8_t mRxBuf0[CONSOLE_MAX_STRING]; ///< Буфер для приема данных для консоли.
	uint16_t mIndex0=0;///< Размер данных в буфере для приема данных для консоли.
	
    CJsonParser mJson;					///< Данные парсера.
	onJsonCmdEvent* onCmd = nullptr;	///< Обработка события команды json.
	/// Задекорированная сallback функция на команду json.
	/*!
	  \param[in] json аргумент команды.
	  \return 0 в случае успеха, иначе ошибка
	*/
	int onJsonCmd(const char* json);

	onConectEvent* onConnect = nullptr;	///< Обработка события подключения

public:
	/// Единственный экземпляр класса.
	/*!
	  \return Указатель на CConsole
	*/
	static CUsbUart* Instance()
	{
		static CUsbUart theSingleInstance;
		return &theSingleInstance;
	}
 	/// Деструктор.
  	virtual ~CUsbUart(){};

	/// Начальная инициализация.
	/*!
	  \param[in] queueLength Максимальная длина очереди сообщений.
	  \param[in] coreID Ядро CPU (0,1).
	*/
    void init(UBaseType_t queueLength = 30, BaseType_t coreID = 1)
    {
        CBaseTask::init("cdc", 4096, 0, queueLength, coreID);
    };

	/// Послать сообщение в задачу.
	/*!
	  \param[in] msg Указатель на сообщение.
	  \param[in] xTicksToWait Время ожидания в тиках.
	  \param[in] free вернуть память в кучу в случае неудачи.
	  \return true в случае успеха.
	*/
	inline bool sendMessage(STaskMessage* msg,TickType_t xTicksToWait=0, bool free=false) override
	{
		return CBaseTask::sendMessage(msg, 0, xTicksToWait, free);
	};

	/// Послать сообщение в задачу из прерывания.
	/*!
	  \param[in] msg Указатель на сообщение.
	  \param[out] pxHigherPriorityTaskWoken Флаг переключения задач.
	  \return true в случае успеха.
	*/
	inline bool sendMessageFromISR(STaskMessage* msg,BaseType_t *pxHigherPriorityTaskWoken) override
	{
		return CBaseTask::sendMessageFromISR(msg, pxHigherPriorityTaskWoken, 0);
	};

 	/// Запуск драйвера.
	/*!
	  \param[in] func Обработчик json команды.
	  \param[in] connect Обработчик подключения.
	  \param[in] queueLength Максимальная длина очереди сообщений.
	*/
	void start(onJsonCmdEvent* func, onConectEvent* connect = nullptr, BaseType_t queueLength = 30);
};

#endif // CONFIG_TINYUSB_CDC_ENABLED

#endif //CUSBUART_H
