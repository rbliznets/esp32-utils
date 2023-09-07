/*!
	\file
	\brief Класс для вывода отладочной информации в файл.
	\authors Близнец Р.А.
	\version 1.0.0.0
	\date 05.09.2023

	Один объект на приложение.
    Необходим чтобы не блокировать отлаживаемую задачу.
*/

#include "CSpiffsTraceTask.h"
#include <cstring>
#include <string>
#include "esp_system.h"
#include "esp_log.h"
#include <dirent.h>
#include <list>

#ifdef CONFIG_TRACE_AUTO_RESET
#define AUTO_TIMER CONFIG_TRACE_AUTO_RESET
#else
#define AUTO_TIMER true
#endif

static const char *TAG = "logfile";

void CSpiffsTraceTask::printHeader(uint64_t time, uint32_t n)
{
    uint64_t res=time/n;
#if (CONFIG_TRACE_USEC == 1)
    std::fprintf(mLogFile,"(+%liusec)",(long)res);
#else
    if(res >= 10000000)
    {
        std::fprintf(mLogFile,"(+%lisec)",(long)(res/1000000));
    }
    else
    {
        if(res<10000)
        {
            if(res < 10)
            {
                double f = time/(double)n;
                std::fprintf(mLogFile,"(+%linsec)",(long)(f*1000));
            }
            else
            {
                std::fprintf(mLogFile,"(+%liusec)",(long)res);
            }
        }
        else
        {
            std::fprintf(mLogFile,"(+%limsec)",(long)(res/1000));
        }
	}
#endif
}

void CSpiffsTraceTask::run()
{
	STaskMessage msg;
	std::string str="/spiffs/_log";

#ifdef CONFIG_UTILS_SPIFFS_FILES_DEL
	std::list<uint8_t> logs;
	struct dirent *entry;
	DIR* dp = opendir("/spiffs");
	if (dp == nullptr) 
	{
		ESP_LOGE(TAG, "Failed to open dir /spiffs");
		closedir(dp);
		return;
	}
	while((entry = readdir(dp)))
	{
		if(std::memcmp(entry->d_name,"_log",4) == 0)
		{
			entry->d_name[std::strlen(entry->d_name)-3] = 0;
			logs.push_back(atoi(&(entry->d_name[4])));
       		//ESP_LOGI(TAG,"#%d", logs.back());
		}
	}
	closedir(dp);
	if(logs.size() == 0)
	{
		str += "0.txt";
	}
	else
	{
		char mst[4];
		sprintf(mst, "%d", logs.back()+1);
		str += mst;
		str += ".txt";
		while(logs.size() >= CONFIG_UTILS_SPIFFS_FILES_COUNT)
		{
			std::string str2="/spiffs/_log";
			sprintf(mst, "%d", logs.front());
			str2 += mst;
			str2 += ".txt";
			if( std::remove(str2.c_str()) == 0)
			{
				ESP_LOGI(TAG,"rm %s", str2.c_str());
			}
			logs.pop_front();
		}
		logs.clear();
	}
#else
	str += "0.txt";
#endif // CONFIG_UTILS_SPIFFS_FILES_DEL
    ESP_LOGI(TAG,"file %s", str.c_str());

	mLogFile = nullptr;
	while(getMessage(&msg,portMAX_DELAY))
	{
		if(mLogFile == nullptr) mLogFile  = std::fopen(str.c_str(), "a");
		switch(msg.msgID)
		{
		case MSG_TRACE_STRING:
			printString((char*)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_STOP_TIME:
			printStop((char*)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_PRINT_STRING:
			std::fprintf(mLogFile,(char*)msg.msgBody);
			vPortFree(msg.msgBody);
			std::fprintf(mLogFile,"\n");
			break;
		case MSG_TRACE_STRING_REBOOT:
			printString((char*)msg.msgBody);
			vPortFree(msg.msgBody);
			std::fprintf(mLogFile,"trace reboot...\n");
			// vPortFree(msg.msgBody);
			fflush(mLogFile);
			std::fclose(mLogFile);
			esp_restart();
			break;
		case MSG_TRACE_UINT8:
			printData8h((char*)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_TRACE2_UINT8:
			printData8h_2((char*)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_TRACE_INT8:
			printData8((char*)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_TRACE2_INT8:
			printData8_2((char*)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_TRACE_UINT16:
			printData16h((char*)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_TRACE2_UINT16:
			printData16h_2((char*)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_TRACE_INT16:
			printData16((char*)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_TRACE2_INT16:
			printData16_2((char*)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_TRACE_UINT32:
			printData32h((char*)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_TRACE2_UINT32:
			printData32h_2((char*)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_TRACE_INT32:
			printData32((char*)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_TRACE2_INT32:
			printData32_2((char*)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		default:
			std::fprintf(mLogFile,"CSpiffsTraceTask unknown message %d\n", msg.msgID);
			break;
		}
		std::fflush(mLogFile);
		//ESP_LOGI(TAG,"msg %d",msg.msgID);
	}
	std::fclose(mLogFile);
}

void CSpiffsTraceTask::printData32_2(char* data)
{
    uint64_t* res=(uint64_t*)data;
    uint32_t* size=(uint32_t*)&data[8];
	uint32_t* pdata=(uint32_t*)(data[8+4]+data[8+4+1]*256+data[8+4+2]*256*256+data[8+4+3]*256*256*256);
    char* strError = &data[8+4+4];

    printHeader(*res);

	std::fprintf(mLogFile,"%s %ld:", strError, *size);
	std::fprintf(mLogFile," %d",(int)pdata[0]);
	for(int16_t i=1; i < *size; i++)
	{
		std::fprintf(mLogFile,",%d",(int)pdata[i]);
	}
	std::fprintf(mLogFile,"\n");
}

void CSpiffsTraceTask::printData32h_2(char* data)
{
    uint64_t* res=(uint64_t*)data;
    uint32_t* size=(uint32_t*)&data[8];
	uint32_t* pdata=(uint32_t*)(data[8+4]+data[8+4+1]*256+data[8+4+2]*256*256+data[8+4+3]*256*256*256);
    char* strError = &data[8+4+4];

    printHeader(*res);

	std::fprintf(mLogFile,"%s %ld:", strError, *size);
	std::fprintf(mLogFile," 0x%08x",(int)pdata[0]);
	for(int16_t i=1; i < *size; i++)
	{
		std::fprintf(mLogFile,",0x%08x",(int)pdata[i]);
	}
	std::fprintf(mLogFile,"\n");
}

void CSpiffsTraceTask::printData32h(char* data)
{
    uint64_t* res=(uint64_t*)data;
    uint32_t* size=(uint32_t*)&data[8];
	uint32_t* pdata=(uint32_t*)&data[8+4];
    char* strError = &data[8+4+((*size)*sizeof(uint32_t))];

    printHeader(*res);

	std::fprintf(mLogFile,"%s %ld:", strError, *size);
	std::fprintf(mLogFile," 0x%08x",(int)pdata[0]);
	for(int16_t i=1; i < *size; i++)
	{
		std::fprintf(mLogFile,",0x%08x",(int)pdata[i]);
	}
	std::fprintf(mLogFile,"\n");
}

void CSpiffsTraceTask::printData32(char* data)
{
    uint64_t* res=(uint64_t*)data;
    uint32_t* size=(uint32_t*)&data[8];
	int32_t* pdata=(int32_t*)&data[8+4];
    char* strError = &data[8+4+((*size)*sizeof(uint32_t))];

    printHeader(*res);

	std::fprintf(mLogFile,"%s %ld:", strError, *size);
	std::fprintf(mLogFile," %d",(int)pdata[0]);
	for(int16_t i=1; i < *size; i++)
	{
		std::fprintf(mLogFile,",%d",(int)pdata[i]);
	}
	std::fprintf(mLogFile,"\n");
}

void CSpiffsTraceTask::printData16h(char* data)
{
    uint64_t* res=(uint64_t*)data;
    uint32_t* size=(uint32_t*)&data[8];
	uint16_t* pdata=(uint16_t*)&data[8+4];
    char* strError = &data[8+4+((*size)*sizeof(uint16_t))];

    printHeader(*res);

	std::fprintf(mLogFile,"%s %ld:", strError, *size);
	std::fprintf(mLogFile," 0x%04x",pdata[0]);
	for(int16_t i=1; i < *size; i++)
	{
		std::fprintf(mLogFile,",0x%04x",pdata[i]);
	}
	std::fprintf(mLogFile,"\n");
}

void CSpiffsTraceTask::printData16h_2(char* data)
{
    uint64_t* res=(uint64_t*)data;
    uint32_t* size=(uint32_t*)&data[8];
	uint16_t* pdata=(uint16_t*)(data[8+4]+data[8+4+1]*256+data[8+4+2]*256*256+data[8+4+3]*256*256*256);
    char* strError = &data[8+4+4];

    printHeader(*res);

	std::fprintf(mLogFile,"%s %ld:", strError, *size);
	std::fprintf(mLogFile," 0x%04x",pdata[0]);
	for(int16_t i=1; i < *size; i++)
	{
		std::fprintf(mLogFile,",0x%04x",pdata[i]);
	}
	std::fprintf(mLogFile,"\n");
}

void CSpiffsTraceTask::printData16(char* data)
{
    uint64_t* res=(uint64_t*)data;
    uint32_t* size=(uint32_t*)&data[8];
	int16_t* pdata=(int16_t*)&data[8+4];
    char* strError = &data[8+4+((*size)*sizeof(uint16_t))];

    printHeader(*res);

	std::fprintf(mLogFile,"%s %ld:", strError, *size);
	std::fprintf(mLogFile," %d",pdata[0]);
	for(int16_t i=1; i < *size; i++)
	{
		std::fprintf(mLogFile,",%d",pdata[i]);
	}
	std::fprintf(mLogFile,"\n");
}

void CSpiffsTraceTask::printData16_2(char* data)
{
    uint64_t* res=(uint64_t*)data;
    uint32_t* size=(uint32_t*)&data[8];
	uint16_t* pdata=(uint16_t*)(data[8+2]+data[8+4+1]*256+data[8+4+2]*256*256+data[8+4+3]*256*256*256);
    char* strError = &data[8+4+4];

    printHeader(*res);

	std::fprintf(mLogFile,"%s %ld:", strError, *size);
	std::fprintf(mLogFile," %d",pdata[0]);
	for(int16_t i=1; i < *size; i++)
	{
		std::fprintf(mLogFile,",%d",pdata[i]);
	}
	std::fprintf(mLogFile,"\n");
}

void CSpiffsTraceTask::printData8h(char* data)
{
    uint64_t* res=(uint64_t*)data;
    uint32_t* size=(uint32_t*)&data[8];
	uint8_t* pdata=(uint8_t*)&data[8+4];
    char* strError = &data[8+4+((*size)*sizeof(uint8_t))];

    printHeader(*res);

	std::fprintf(mLogFile,"%s %ld:", strError, *size);
	std::fprintf(mLogFile," 0x%02x",pdata[0]);
	for(int16_t i=1; i < *size; i++)
	{
		std::fprintf(mLogFile,",0x%02x",pdata[i]);
	}
	std::fprintf(mLogFile,"\n");
}

void CSpiffsTraceTask::printData8h_2(char* data)
{
    uint64_t* res=(uint64_t*)data;
    uint32_t* size=(uint32_t*)&data[8];
	uint8_t* pdata=(uint8_t*)(data[8+4]+data[8+4+1]*256+data[8+4+2]*256*256+data[8+4+3]*256*256*256);
    char* strError = &data[8+4+4];

    printHeader(*res);

	std::fprintf(mLogFile,"%s %ld:", strError, *size);
	std::fprintf(mLogFile," 0x%02x",pdata[0]);
	for(int16_t i=1; i < *size; i++)
	{
		std::fprintf(mLogFile,",0x%02x",pdata[i]);
	}
	std::fprintf(mLogFile,"\n");
}

void CSpiffsTraceTask::printData8(char* data)
{
    uint64_t* res=(uint64_t*)data;
    uint32_t* size=(uint32_t*)&data[8];
	int8_t* pdata=(int8_t*)&data[8+4];
    char* strError = &data[8+4+((*size)*sizeof(uint8_t))];

    printHeader(*res);

	std::fprintf(mLogFile,"%s %ld:", strError, *size);
	std::fprintf(mLogFile," %d",pdata[0]);
	for(int16_t i=1; i < *size; i++)
	{
		std::fprintf(mLogFile,",%d",pdata[i]);
	}
	std::fprintf(mLogFile,"\n");
}

void CSpiffsTraceTask::printData8_2(char* data)
{
    uint64_t* res=(uint64_t*)data;
    uint32_t* size=(uint32_t*)&data[8];
	uint8_t* pdata=(uint8_t*)(data[8+4]+data[8+4+1]*256+data[8+4+2]*256*256+data[8+4+3]*256*256*256);
    char* strError = &data[8+4+4];

    printHeader(*res);

	std::fprintf(mLogFile,"%s %ld:", strError, *size);
	std::fprintf(mLogFile," %d",pdata[0]);
	for(int16_t i=1; i < *size; i++)
	{
		std::fprintf(mLogFile,",%d",pdata[i]);
	}
	std::fprintf(mLogFile,"\n");
}

void CSpiffsTraceTask::printString(char* data)
{
    uint64_t* res=(uint64_t*)data;
    int32_t* errCode=(int32_t*)&data[8];
    char* strError = &data[12];

    printHeader(*res);
	std::fprintf(mLogFile,"->%d:%s\n",(int)(*errCode), strError);
}

void CSpiffsTraceTask::printStop(char* data)
{
    uint64_t* x=(uint64_t*)data;
    int32_t* n=(int32_t*)&data[8];
    char* str = &data[12];

    printHeader(*x, *n);

	std::fprintf(mLogFile," %s\n",str);
}

