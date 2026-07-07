cmake_minimum_required(VERSION 3.20)

project(nuedc_control_template LANGUAGES C ASM)

set(APP_TARGET ${PROJECT_NAME})
set(CMAKE_EXECUTABLE_SUFFIX ".out")

option(NUEDC_NO_ENCODER "Build the template without wheel encoder feedback" OFF)
option(NUEDC_USE_K230 "Enable the optional K230 vision UART BSP" OFF)

if(DEFINED ENV{MSPM0_SDK_ROOT} AND NOT "$ENV{MSPM0_SDK_ROOT}" STREQUAL "")
  set(MSPM0_SDK_ROOT "$ENV{MSPM0_SDK_ROOT}" CACHE PATH "Path to the MSPM0 SDK root")
else()
  set(MSPM0_SDK_ROOT "" CACHE PATH "Path to the MSPM0 SDK root")
endif()

function(find_first_match output_var)
  set(found "")
  foreach(pattern IN LISTS ARGN)
    file(GLOB_RECURSE matches LIST_DIRECTORIES false "${pattern}")
    list(SORT matches)
    if(matches)
      list(GET matches 0 found)
      break()
    endif()
  endforeach()
  set(${output_var} "${found}" PARENT_SCOPE)
endfunction()

if(NOT MSPM0_SDK_ROOT)
  foreach(candidate IN ITEMS
    "$ENV{HOME}/ti/mspm0_sdk_2_10_00_04"
    "$ENV{HOME}/ti/mspm0_sdk_2_10_00_04/mspm0_sdk_2_10_00_04"
    "$ENV{HOME}/ti/mspm0_sdk_2_07_00_05"
    "$ENV{HOME}/ti/mspm0_sdk_2_07_00_05/mspm0_sdk_2_07_00_05"
    "/opt/ti/mspm0_sdk"
  )
    if(EXISTS "${candidate}/.metadata/product.json")
      set(MSPM0_SDK_ROOT "${candidate}" CACHE PATH "Path to the MSPM0 SDK root" FORCE)
      break()
    endif()
  endforeach()
endif()

if(NOT MSPM0_SDK_ROOT OR NOT EXISTS "${MSPM0_SDK_ROOT}/.metadata/product.json")
  message(FATAL_ERROR "MSPM0 SDK not found. Install it and set MSPM0_SDK_ROOT.")
endif()

find_first_match(MSPM0_STARTUP_FILE
  "${MSPM0_SDK_ROOT}/source/ti/devices/msp/m0p/startup_system_files/gcc/startup_mspm0g350x_gcc.c"
  "${MSPM0_SDK_ROOT}/examples/nortos/LP_MSPM0G3507/*/gcc/startup_mspm0g350x_gcc.c"
)

set(MSPM0_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/device_linker.lds")
set(MSPM0_DRIVERLIB_A
  "${MSPM0_SDK_ROOT}/source/ti/driverlib/lib/gcc/m0p/mspm0g1x0x_g3x0x/driverlib.a")

if(NOT EXISTS "${MSPM0_STARTUP_FILE}")
  message(FATAL_ERROR "Unable to find MSPM0 GCC startup file.")
endif()
if(NOT EXISTS "${MSPM0_LINKER_SCRIPT}")
  message(FATAL_ERROR "Missing linker script: ${MSPM0_LINKER_SCRIPT}")
endif()
if(NOT EXISTS "${MSPM0_DRIVERLIB_A}")
  message(FATAL_ERROR "Missing driverlib.a: ${MSPM0_DRIVERLIB_A}")
endif()

set(FREERTOS_SOURCES
  ${MSPM0_SDK_ROOT}/kernel/freertos/Source/tasks.c
  ${MSPM0_SDK_ROOT}/kernel/freertos/Source/list.c
  ${MSPM0_SDK_ROOT}/kernel/freertos/Source/queue.c
  ${MSPM0_SDK_ROOT}/kernel/freertos/Source/portable/MemMang/heap_4.c
  ${MSPM0_SDK_ROOT}/kernel/freertos/Source/portable/GCC/ARM_CM0/port.c
  ${MSPM0_SDK_ROOT}/kernel/freertos/Source/portable/GCC/ARM_CM0/portasm.c
)

set(TDK_ICM45686_SOURCES
  third_party/tdk_icm45686/src/imu/inv_imu_driver.c
  third_party/tdk_icm45686/src/imu/inv_imu_transport.c
)

set(APP_SOURCES
  main.c
  ti_msp_dl_config.c
  ${MSPM0_STARTUP_FILE}
  ${FREERTOS_SOURCES}
  drivers/drv_gpio.c
  drivers/drv_hbridge_motor.c
  drivers/drv_i2c.c
  drivers/drv_line_sensor.c
  drivers/drv_pwm.c
  drivers/drv_quad_encoder.c
  drivers/drv_time.c
  drivers/drv_uart.c
  devices/dev_icm45686.c
  devices/dev_line_sensor.c
  devices/dev_motor.c
  devices/dev_vision.c
  bsp/bsp_adc.c
  bsp/bsp_encoder.c
  bsp/bsp_imu.c
  bsp/bsp_motor.c
  bsp/bsp_uart.c
  middleware/control/chassis.c
  middleware/control/line_tracker.c
  middleware/filter.c
  middleware/pid.c
  middleware/rtt_log.c
  middleware/telemetry.c
  app/app_main.c
  ${TDK_ICM45686_SOURCES}
)

if(NUEDC_USE_K230)
  list(APPEND APP_SOURCES bsp/bsp_k230.c)
endif()

add_executable(${APP_TARGET} ${APP_SOURCES})

target_include_directories(${APP_TARGET} PRIVATE
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/config"
  "${CMAKE_SOURCE_DIR}/app"
  "${CMAKE_SOURCE_DIR}/bsp"
  "${CMAKE_SOURCE_DIR}/devices"
  "${CMAKE_SOURCE_DIR}/drivers"
  "${CMAKE_SOURCE_DIR}/middleware"
  "${CMAKE_SOURCE_DIR}/third_party/tdk_icm45686/src"
  "${MSPM0_SDK_ROOT}/source"
  "${MSPM0_SDK_ROOT}/source/third_party/CMSIS/Core/Include"
  "${MSPM0_SDK_ROOT}/kernel/freertos/Source/include"
  "${MSPM0_SDK_ROOT}/kernel/freertos/Source/portable/GCC/ARM_CM0"
)

target_compile_definitions(${APP_TARGET} PRIVATE
  __MSPM0G3507__
)

if(NUEDC_NO_ENCODER)
  target_compile_definitions(${APP_TARGET} PRIVATE NUEDC_NO_ENCODER=1)
endif()
if(NUEDC_USE_K230)
  target_compile_definitions(${APP_TARGET} PRIVATE NUEDC_USE_K230=1)
endif()

target_compile_options(${APP_TARGET} PRIVATE
  -mcpu=cortex-m0plus
  -mthumb
  -ffunction-sections
  -fdata-sections
  -fno-common
  -Wall
  -Wextra
  -Wno-unused-parameter
  -Og
  -g2
  -gdwarf-4
)

target_link_options(${APP_TARGET} PRIVATE
  -mcpu=cortex-m0plus
  -mthumb
  -nostartfiles
  -T${MSPM0_LINKER_SCRIPT}
  -Wl,-e,Reset_Handler
  -Wl,--gc-sections
  -Wl,-Map=${CMAKE_BINARY_DIR}/${APP_TARGET}.map
  --specs=nano.specs
  --specs=nosys.specs
  -Wl,-u,_printf_float
)

target_link_libraries(${APP_TARGET} PRIVATE
  "${MSPM0_DRIVERLIB_A}"
  m
)

find_program(ARM_SIZE arm-none-eabi-size)
find_program(ARM_OBJCOPY arm-none-eabi-objcopy)

add_custom_command(TARGET ${APP_TARGET} POST_BUILD
  COMMAND "${ARM_SIZE}" "$<TARGET_FILE:${APP_TARGET}>"
  COMMAND "${ARM_OBJCOPY}" -O ihex "$<TARGET_FILE:${APP_TARGET}>" "${CMAKE_BINARY_DIR}/${APP_TARGET}.hex"
  COMMAND "${ARM_OBJCOPY}" -O binary "$<TARGET_FILE:${APP_TARGET}>" "${CMAKE_BINARY_DIR}/${APP_TARGET}.bin"
  VERBATIM
)
