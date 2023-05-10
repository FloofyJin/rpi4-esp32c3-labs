idf_component_register(SRCS "helloworld.cpp" 
                            "DFRobot_LCD.cpp" 
                            "i2c_master_init.c"
                    INCLUDE_DIRS ".")

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-const-variable)