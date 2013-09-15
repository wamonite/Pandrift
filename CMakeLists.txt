################################################################
# Pandrift
# Copyright (c) 2013 Warren Moore
#
# This software may be redistributed under the terms of the MIT License.
# See the file LICENSE for details.
################################################################

PROJECT(Pandrift)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF (APPLE)
  INCLUDE_DIRECTORIES(/System/Library/Frameworks)
  FIND_LIBRARY(APPSERVICES_LIBRARY ApplicationServices)
  FIND_LIBRARY(IOKIT_LIBRARY IOKit)
  MARK_AS_ADVANCED (
    APPSERVICES_LIBRARY
    IOKIT_LIBRARY
  )
  SET(PANDRIFT_EXTRA_LIBS ${APPSERVICES_LIBRARY} ${IOKIT_LIBRARY})
ENDIF (APPLE)

INCLUDE_DIRECTORIES(/Developer/Panda3d/include)
INCLUDE_DIRECTORIES(/opt/local/Library/Frameworks/Python.framework/Versions/2.6/include/python2.6)
INCLUDE_DIRECTORIES(/Users/robowaz/Development/install/boost_1_53_0/include)
INCLUDE_DIRECTORIES(/Users/robowaz/Development/src/LibOVR/Include)

LINK_DIRECTORIES(/Developer/Panda3D/lib)
LINK_DIRECTORIES(/Users/robowaz/Development/install/boost_1_53_0/lib)
LINK_DIRECTORIES(/Users/robowaz/Development/src/LibOVR.eclipse)

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(example)
