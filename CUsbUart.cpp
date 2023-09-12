/*!
	\file
	\brief Класс обертка для tinyUSB СDС.
	\authors Близнец Р.А.
	\version 1.0.0.0

	Один объект на приложение.
*/

#include "sdkconfig.h"
#ifdef CONFIG_TINYUSB_CDC_ENABLED

#include "CUsbUart.h"
#include "esp_log.h"
#include "CTrace.h"
#include <cstring>

#include "esp_sleep.h"

#define MSG_END_TASK 		(0)
#define MSG_CONSOLE_STOP	(2098)
#define MSG_CONSOLE_START	(2099)
#define MSG_CONSOLE_RX 		(2100)

bool CUsbUart::mConsole0 = false;
static const char *TAG = "CUsbUart";

void CUsbUart::cdc_rx_callback(int itf, cdcacm_event_t *event)
{
    if(itf == TINYUSB_CDC_ACM_0)
    {
        CUsbUart::Instance()->console_rx((tinyusb_cdcacm_itf_t)itf);
    }
}

void CUsbUart::console_rx(tinyusb_cdcacm_itf_t itf)
{
    size_t rx_size = 0;
    esp_err_t ret;
    STaskMessage msg;

    ret = tinyusb_cdcacm_read(itf, &mRxBuf0[mIndex0], CONSOLE_MAX_STRING-mIndex0, &rx_size);
    if (ret == ESP_OK) 
    {
        if((mIndex0+rx_size) == CONSOLE_MAX_STRING)
        {
            TDEC("CUsbUart console Read overload",-1);
            mIndex0 = 0;
        }
        else
        {
            for(int i = 0; i < rx_size; i++)
            {
                if(mRxBuf0[mIndex0] == 0x0d)
                {
                    mRxBuf0[mIndex0] = 0;
                    mIndex0++;
                    uint8_t* dt = allocNewMsg(&msg,MSG_CONSOLE_RX,mIndex0);
                    std::memcpy(dt,mRxBuf0,mIndex0);
                    sendMessage(&msg,1,true);

                    if(i != (rx_size-1))
                    {
                        std::memmove(mRxBuf0,&mRxBuf0[mIndex0],rx_size-i-1);
                    }
                    mIndex0 = 0;
                }
                else  mIndex0++;
            }
        }
    } 
    else 
    {
        TDEC("CUsbUart console Read error",-2);
    }
}

void CUsbUart::cdc_line_state_changed_callback(int itf, cdcacm_event_t *event)
{
	int dtr = event->line_state_changed_data.dtr;
    //int rts = event->line_state_changed_data.rts;
    if((dtr == 1) && (itf == TINYUSB_CDC_ACM_0))
    {
        if(!CUsbUart::Instance()->mConsole0)
        {
            esp_tusb_init_console(itf);
            CUsbUart::Instance()->mConsole0=true;
        }
        CUsbUart::Instance()->sendCmd(MSG_CONSOLE_START);
    }
    else if((dtr == 0) && (itf == TINYUSB_CDC_ACM_0) && (CUsbUart::Instance()->mConsole0))
    {
        // esp_tusb_deinit_console(itf);
        // CUsbUart::Instance()->mConsole0=false;
        CUsbUart::Instance()->sendCmd(MSG_CONSOLE_STOP);
   }
}

void CUsbUart::start(onJsonCmdEvent* func, onConectEvent* connect, BaseType_t queueLength)
{
    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = nullptr,
        .string_descriptor = nullptr,
        .external_phy = false,
        .configuration_descriptor = nullptr,
        .self_powered = false,
        .vbus_monitor_io = 0
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    tinyusb_config_cdcacm_t acm_cfg = {
        .usb_dev = TINYUSB_USBDEV_0,
        .cdc_port = TINYUSB_CDC_ACM_0,
        .rx_unread_buf_sz = 64,
        .callback_rx = &cdc_rx_callback, // the first way to register a callback
        .callback_rx_wanted_char = nullptr,
        .callback_line_state_changed = nullptr,
        .callback_line_coding_changed = nullptr
    };

    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));
    /* the second way to register a callback */
    ESP_ERROR_CHECK(tinyusb_cdcacm_register_callback(
                        TINYUSB_CDC_ACM_0,
                        CDC_EVENT_LINE_STATE_CHANGED,
                        &cdc_line_state_changed_callback));

#if (CONFIG_TINYUSB_CDC_COUNT > 1)
    acm_cfg.cdc_port = TINYUSB_CDC_ACM_1;
    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));
    ESP_ERROR_CHECK(tinyusb_cdcacm_register_callback(
                        TINYUSB_CDC_ACM_1,
                        CDC_EVENT_LINE_STATE_CHANGED,
                        &cdc_line_state_changed_callback));
#endif

   	onCmd = func;
    onConnect = connect;
    CBaseTask::init("cdc", 4096, CONFIG_TINYUSB_TASK_PRIORITY-1, queueLength, CONFIG_TINYUSB_TASK_AFFINITY);
}

void CUsbUart::run()
{
	STaskMessage msg;
    int t1;
    const char* json;
    char* pch;

	while(getMessage(&msg,portMAX_DELAY))
	{
		switch(msg.msgID)
		{
            case MSG_CONSOLE_RX:
                if(onCmd != nullptr)
                {
                    json = (const char*)msg.msgBody;
                    LOG(json);
                    pch=strchr(json,'{');
                    if(pch != nullptr)
                    {
                        t1=mJson.parse((const char*)pch);
                        if(t1 != 1)
                        {
                            LOG("json failed");
                        }
                        else
                        {
                            onCmd(&mJson);
                        }
                    }
                    else
                    {
                        LOG("json failed");
                    }
                }

                vPortFree(msg.msgBody);
                break;
            case MSG_CONSOLE_START:
                if(onCmd != nullptr) LOG("enter json command");
                if(onConnect != nullptr) onConnect(true);
                break;
            case MSG_CONSOLE_STOP:
                if(onConnect != nullptr) onConnect(false);
                // esp_sleep_enable_timer_wakeup((5 * 1000 * 1000));
                // esp_light_sleep_start();
                // esp_deep_sleep_start();
                break;
            case MSG_END_TASK:
                return;
			default:
                ESP_LOGW(TAG, "unknown message %d:", msg.msgID);
                break;
		}
	}
}

#endif // CONFIG_TINYUSB_CDC_ENABLED
