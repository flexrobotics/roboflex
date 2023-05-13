# roboflex.visualization

A thin wrapper of various display nodes over SDL. For now just supplies
one node: RGBImageTV. It can display rgb tensors of the form (H,W,3) uint8.

## System requirements

* libsdl: SDL (Simple Directmedia Layer)

        # search for existing libsdl packages
        dpkg -l | grep sdl # -> shows nothing

        # install libsdl
        sudo apt-get install libsdl2-dev

        # search for existing libsdl packages again
        dpkg -l | grep sdl # -> shows:
        ii  libsdl2-2.0-0:amd64                        2.0.20+dfsg-2ubuntu1.22.04.1                                        amd64        Simple DirectMedia Layer
        ii  libsdl2-dev:amd64                          2.0.20+dfsg-2ubuntu1.22.04.1                                        amd64        Simple DirectMedia Layer development files

        THIS, for me on ubuntu22, puts headers in /usr/include/SDL2 and libs in /lib/x86_64-linux-gnu/libSDL2-2.0.so

        # to remove it:
        sudo apt-get -y purge libsdl2-dev