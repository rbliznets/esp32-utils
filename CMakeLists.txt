idf_component_register(SRCS "CConsole.cpp" 
                    "CJsonParser.cpp" 
                    "CUsbUart.cpp"
                    "utils.cpp"
                    "CSpiffsTraceTask.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES jsmn task console esp_tinyusb spiffs)
