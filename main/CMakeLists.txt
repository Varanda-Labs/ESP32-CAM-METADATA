set(srcs "main.c"
	     "connect_wifi.c"
)

idf_component_register(SRCS ${srcs}
    INCLUDE_DIRS "." 
    "../components/exif"
)

add_compile_definitions(
    WIFI_SSID="$ENV{WIFI_SSID}"
    WIFI_PASSWORD="$ENV{WIFI_PASSWORD}"
)
