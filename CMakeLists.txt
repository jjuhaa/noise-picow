add_executable(noise
	noise.c
	mqtt.c
	ssd1306_i2c.c
    )

target_sources(noise PRIVATE 
	${PICO_SDK_PATH}/lib/lwip/src/apps/mqtt/mqtt.c
	)

target_link_libraries(noise
	pico_stdlib
	hardware_adc
	pico_cyw43_arch_lwip_threadsafe_background 
	hardware_i2c
	)

# create map/bin/hex file etc.
pico_add_extra_outputs(noise)

# enable usb output, disable uart output
pico_enable_stdio_usb(noise 1)
pico_enable_stdio_uart(noise 0)

target_include_directories(noise PRIVATE 
	${CMAKE_CURRENT_LIST_DIR} ${PROJECT_SOURCE_DIR}
	)