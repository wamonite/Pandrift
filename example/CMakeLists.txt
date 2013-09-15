################################################################
# Pandrift
# Copyright (c) 2013 Warren Moore
#
# This software may be redistributed under the terms of the MIT License.
# See the file LICENSE for details.
################################################################

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)

SET(PANDRIFT_EXAMPLE_HEADERS
  world.hh
)

SET(PANDRIFT_EXAMPLE_SOURCES
  world.cc
  example.cc
)

ADD_EXECUTABLE(example ${PANDRIFT_EXAMPLE_SOURCES} ${PANDRIFT_EXAMPLE_HEADERS})

SET_TARGET_PROPERTIES(example PROPERTIES COMPILE_FLAGS -fPIC)

TARGET_LINK_LIBRARIES(example p3framework panda pandafx pandaexpress p3dtoolconfig p3dtool p3pystub p3direct ovr pandrift ${PANDRIFT_EXTRA_LIBS})
