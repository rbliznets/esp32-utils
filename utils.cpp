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

static const char *TAG = "utils";

#ifdef CONFIG_UTILS_SPIFFS_INIT
void spiffs_init()
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = nullptr,
        .max_files = 15,
        .format_if_mount_failed = true};
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

#ifdef CONFIG_UTILS_SPIFFS_CHECK
    ESP_LOGI(TAG, "SPIFFS checking...");
    ret = esp_spiffs_check(conf.partition_label);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
        return;
    }
    else
    {
        ESP_LOGI(TAG, "SPIFFS_check() successful");
    }
#endif // CONFIG_UTILS_SPIFFS_CHECK

    esp_spiffs_gc(conf.partition_label, 0x100000);

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s). Formatting...", esp_err_to_name(ret));
        esp_spiffs_format(conf.partition_label);
        return;
    }
    else
    {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}
#endif // CONFIG_UTILS_SPIFFS_INIT

void init_utils()
{
#ifdef CONFIG_UTILS_SPIFFS_INIT
    spiffs_init();
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
