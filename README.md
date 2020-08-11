# PAPERVIEW

Paperview is a high performance animated desktop background setter for Linux and X11.

![](screenshot.png)

Video of the above screenshot: https://www.youtube.com/watch?v=6ZTiA885bWM

## Build

    make # NOTE: SDL2 is required

## Use

    ./paperview FOLDER SPEED

A lower SPEED number will result in a faster frame rate. Only BMP files are supported.
Creating your own BMP folders from gifs requires imagemagick. For example, creating
a castle folder from castle.gif which you downloaded:

    mkdir castle
    mv castle.gif castle
    cd castle
    convert -coalesce castle.gif out.bmp
    rm castle.gif

## Performance

Running on a Thinkpad X230 from 2012 at 1920x1080 and 60fps with an integrated Intel GPU:

    intel_gpu_time ./paperview castle 5

    user: 1.904135s, sys: 0.357277s, elapsed: 100.458648s, CPU: 2.3%, GPU: 11.7%

## Known Issues

Picom, Compton (and possibly other compositors) seem to already write to the base root X11 window
which may overwrite the render done by paperview.

## Alternatives

Alternatively, if SDL2 is a problem, or if you are on Windows, two workarounds are currently available:

### Pure X11 (without SDL2)

https://gist.github.com/AlecsFerra/ef1cc008990319f3b676eb2d8aa89903

### Windows 10

https://github.com/TrAyZeN/sdl-wallpaper
