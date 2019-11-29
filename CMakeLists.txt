cmake_minimum_required(VERSION 2.6)

PROJECT(uleddd C)
INCLUDE(GNUInstallDirs)

SET(ULEDD_VERSION 1.0.1)

ADD_DEFINITIONS(-Os -ggdb -Wextra -Wall -Werror --std=gnu99)
ADD_DEFINITIONS(-Wno-unused-parameter -Wmissing-declarations)
ADD_DEFINITIONS(-DULEDD_VERSION="${ULEDD_VERSION}")

IF(UNIT_TESTING)
  ADD_DEFINITIONS(-DULEDD_DEBUG -DUNIT_TESTING)
ENDIF()

SET(CMAKE_C_FLAGS_DEBUG -DULEDD_DEBUG)
SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

FIND_PATH(ubox_include_dir NAMES libubox/usock.h)
FIND_PATH(ubus_include_dir NAMES libubus.h)
INCLUDE_DIRECTORIES(${ubox_include_dir} ${ubus_include_dir})

FIND_LIBRARY(ubox NAMES ubox)
FIND_LIBRARY(ubus NAMES ubus)
FIND_LIBRARY(blobmsg_json NAMES blobmsg_json)
FIND_LIBRARY(json NAMES json-c)

SET(SOURCES main.c)

ADD_LIBRARY(uledd-lib STATIC blob_led.c led.c ubus.c scene.c scene_json.c timer.c)
TARGET_LINK_LIBRARIES(uledd-lib ${blobmsg_json} ${json} ${ubus} ${ubox})

IF(UNIT_TESTING)
  ENABLE_TESTING()
  ADD_SUBDIRECTORY(tests)
ENDIF()

ADD_EXECUTABLE(uledd ${SOURCES})
TARGET_LINK_LIBRARIES(uledd uledd-lib)
INSTALL(TARGETS uledd
	RUNTIME DESTINATION ${CMAKE_INSTALL_SBINDIR}
)
