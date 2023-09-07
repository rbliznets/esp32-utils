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

## Консоль
Инициализация ***CConsole::Instance()->start(jsonParse,exitConsole);***
