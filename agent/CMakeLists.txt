# MIT License Copyright (c) 2021, h1zzz

cmake_minimum_required(VERSION 3.10.0)
project(purewater VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/mbedtls/include
)

if(MSVC)
  add_compile_options(/W3 /WX)
else()
  add_compile_options(-Wall -Wextra -pedantic -Werror -Wwrite-strings)
  if(${CMAKE_C_COMPILER_ID} MATCHES "GNU")
    set(CMAKE_EXE_LINKER_FLAGS "-static -O2")
  endif()
endif()

if(MINGW)
  add_definitions(-D_WIN32_WINNT=_WIN32_WINNT_VISTA)
endif()

add_definitions(-DMBEDTLS_CONFIG_FILE="config.h")
add_subdirectory(${PROJECT_SOURCE_DIR}/mbedtls/library)

set(
  HEADERS
  connection.h
  debug.h
  dns.h
  http.h
  llist.h
  network.h
  platform.h
  proxy_https.h
  proxy_socks.h
  socket.h
  socks.c
  # tunnel_dns.h
  # tunnel_http.h
  url.h
  util.h
)

set(
  SOURCES
  connection.c
  dns.c
  http.c
  llist.c
  main.c
  network.c
  platform.c
  proxy_https.c
  proxy_socks.c
  socket.c
  socks.c
  # tunnel_dns.c
  # tunnel_http.c
  url.c
  util.c
)

set(LIBS mbedtls)

if(MSVC OR MINGW)
  list(APPEND LIBS ws2_32)
  list(APPEND LIBS iphlpapi)
endif()

add_executable(agent ${HEADERS} ${SOURCES})
target_link_libraries(agent PRIVATE ${LIBS})
