################################################################
# Pandrift
# Copyright (c) 2013 Warren Moore
#
# This software may be redistributed under the terms of the MIT License.
# See the file LICENSE for details.
################################################################

SET(PANDRIFT_LIBRARY_HEADERS
  pandrift.hh
  pandrift_rift_manager.hh
  pandrift_display_manager.hh
)

SET(PANDRIFT_LIBRARY_SOURCES
  pandrift.cc
  pandrift_rift_manager.cc
  pandrift_display_manager.cc
)

ADD_LIBRARY(pandrift ${PANDRIFT_LIBRARY_HEADERS} ${PANDRIFT_LIBRARY_SOURCES})
