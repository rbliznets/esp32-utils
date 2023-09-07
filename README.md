# Командная консоль и логирование в SPIFFS
Для добавления в проект в папке компонентов из командной строки запустить:    

    git submodule add https://github.com/rbliznets/esp32-utils utils

Зависимость от следующих компонентов:
- task (https://github.com/rbliznets/esp32-task)
- espressif/jsmn
- espressif/esp_tinyusb
- spressif/tinyusb
  
## Логирование и SPIFFS
Для инициализации настроек из *sdkconfig* вызовите ***init_utils()***.
При включении spiffs инициализируется файловая система. Для этого нужно разметка *partitions.csv*, например (2 мегабайтный раздел):

    # Name,   Type, SubType, Offset,  Size, Flags
    # Note: if you have increased the bootloader size, make sure to update the offsets to avoid overlap
    nvs,      data, nvs,     0x9000,  0x6000,
    phy_init, data, phy,     0xf000,  0x1000,
    factory,  app,  factory, 0x10000, 1M,
    storage,  data, spiffs,  0x200000, 2M,

Для *tasks.json* можно добавить задачу (создает образ файловой системы с файлами из каталога **spiffs** проекта и записывает во флэш):

    {
        "label": "spiffs build and flash (2M,offset 0x200000)",
        "command": "${config:idf.pythonBinPath} ${config:idf.espIdfPathWin}\\components\\spiffs\\spiffsgen.py 0x200000 ${workspaceFolder}\\spiffs ${config:idf.buildPath}\\spiffs.bin;${config:idf.pythonBinPath} -m esptool --chip ${config:idf.adapterTargetName} --port ${config:idf.portWin} --baud ${config:idf.flashBaudRate} write_flash -z 0x200000 ${config:idf.buildPath}\\spiffs.bin",
        "type": "shell",
        "problemMatcher": []
    },

При включении логирования в файл (доступно при включении системы логирования в **task**) в файл *_log#.txt* будут писаться все сообщения из ***TRACE(...)***. Можно выбрать новый файл при каждом включении.
Если время включения неважно и используется запись рекомендуется включить проверку при старте.

**Bug:** spiffs может конфликтовать с PSRAM.

## Консоль
Инициализация ***CConsole::Instance()->start();***
Если включено **spiffs**, то доступны команды для неё. Если задать обработчики команд *json* и *выхода* при инициализации, то будут доступны м соответствующие команды. Полный список команд выдается на запрос **help**.
Пример с обработчиком json и переключение консоли на USB CDC при выходе:

    #include "utils.h"
    
    void jsonParse(CJsonParser* parser)
    {
        int t1,t2,freq;
    
        if(parser->getObject(1, "tx", t2))
        {
            if(parser->getInt(t2, "freq", freq))
        ......

        }

    }

    void exitConsole()
    {
        LOG("close console");
        vTaskDelay(pdMS_TO_TICKS(500));

        CUsbUart::Instance()->start(jsonParse);
    }

    extern "C" void app_main(void)
    {
        init_utils();
        CConsole::Instance()->start(jsonParse,exitConsole);

        ....
    }
