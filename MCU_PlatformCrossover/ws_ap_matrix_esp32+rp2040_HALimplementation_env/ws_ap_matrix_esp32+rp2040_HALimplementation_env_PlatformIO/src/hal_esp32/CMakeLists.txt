FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

idf_component_register(
    SRCS ${app_sources}
    INCLUDE_DIRS 
        "."
    REQUIRES 
        esp_http_server 
        esp_wifi 
        esp_event 
        esp_netif 
        nvs_flash
)