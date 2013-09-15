# Pandrift

Oculus Rift code for Panda3D

[source @ github](https://github.com/wamonite/Pandrift)

## Usage

The example application displays the Panda3D tutorial scene.

* f - Toggle fullscreen
* r - Toggle Rift display
* Escape - Exit

## To Do

* Plenty - this is an early, rough and ready release!
* Improve shader resource finding/loading (so you don't have to run it from the src directory).
* Improve the CMake script to remove hard-coded paths.
* Improve the Rift device code.
    * Properly implement device enumeration/selection.
    * Handle runtime connect/disconnect.
    * Implement as a Panda3D client device.
* Enable runtime switching of display parameters.
* Add lookup texture implementation.

## Build Notes

* Tested on OSX Lion
* Built against LibOVR 0.2.4 (rebuilt for i386 without '-fno-rtti -fno-exceptions')

## License

Copyright (c) 2013 Warren Moore

This software may be redistributed under the terms of the MIT License.
See the file LICENSE for details.

## Contact

          @wamonite     - twitter
           \_______.com - web
    warren____________/ - email
