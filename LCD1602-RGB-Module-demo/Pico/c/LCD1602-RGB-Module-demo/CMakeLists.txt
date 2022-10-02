add_executable(LCD1602_RGB_Module_demo
        main.c
        LCD1602_RGB_Module.c LCD1602_RGB_Module.h)

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(LCD1602_RGB_Module_demo pico_stdlib hardware_i2c)

# create map/bin/hex file etc.
pico_add_extra_outputs(LCD1602_RGB_Module_demo)

# add url via pico_set_program_url
example_auto_set_url(LCD1602_RGB_Module_demo)
