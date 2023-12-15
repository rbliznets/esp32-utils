/*!
    \file
    \brief Инициализация utils.
    \authors Близнец Р.А. (r.bliznets@gmail.com)
    \version 1.0.0.0
*/

#include "utils.h"
#include "esp_spiffs.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "CTrace.h"
#include "CSpiffsTraceTask.h"
#include "CSpiffsSystem.h"

static const char *TAG = "utils";

void init_utils()
{
#ifdef CONFIG_UTILS_SPIFFS_INIT
#ifdef CONFIG_UTILS_SPIFFS_CHECK
    CSpiffsSystem::init(true);
#else
    CSpiffsSystem::init(false);
#endif
#endif // CONFIG_UTILS_SPIFFS_INIT
#ifdef CONFIG_UTILS_SPIFFS_TRACE_TASK
#ifdef CONFIG_UTILS_TRACE_TASK0
    CSpiffsTraceTask::Instance()->init(30, 0);
#else
    CSpiffsTraceTask::Instance()->init(30, 1);
#endif
    ADDLOG(CSpiffsTraceTask::Instance());
#endif // CONFIG_UTILS_SPIFFS_TRACE_TASK
}
