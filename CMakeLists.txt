cmake_minimum_required(VERSION 2.6)

PROJECT(uleddd C)
INCLUDE(GNUInstallDirs)

SET(ULEDD_VERSION 1.0.0)

ADD_DEFINITIONS(-Os -ggdb -Wextra -Wall -Werror --std=gnu99)
ADD_DEFINITIONS(-Wno-unused-parameter -Wmissing-declarations)
ADD_DEFINITIONS(-DULEDD_VERSION="${ULEDD_VERSION}")

SET(CMAKE_C_FLAGS_DEBUG -DULEDD_DEBUG)
SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

FIND_PATH(ubox_include_dir NAMES libubox/usock.h)
FIND_PATH(ubus_include_dir NAMES libubus.h)
INCLUDE_DIRECTORIES(${ubox_include_dir} ${ubus_include_dir})

FIND_LIBRARY(ubox NAMES ubox)
FIND_LIBRARY(ubus NAMES ubus)

SET(SOURCES main.c led.c ubus.c timer.c)
SET(LIBS ${ubus} ${ubox})

ADD_EXECUTABLE(uledd ${SOURCES})
TARGET_LINK_LIBRARIES(uledd ${LIBS})
INSTALL(TARGETS uledd
	RUNTIME DESTINATION ${CMAKE_INSTALL_SBINDIR}
)
