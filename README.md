# PAPERVIEW 

A high performance animated desktop background setter for X11 that won't set your CPU on fire,
drain your laptop battery, or lower ya vidya game FPS.

![](http://i3.ytimg.com/vi/zzy8qFKfwCQ/hqdefault.jpg)

Video demo: https://www.youtube.com/watch?v=zzy8qFKfwCQ

Running on a Thinkpad X230 from 2012 at 1920x1080 and 60fps:

    intel_gpu_time ./paperview city 3

    user: 1.904135s, sys: 0.357277s, elapsed: 100.458648s, CPU: 2.3%, GPU: 11.7%

## Build

    pacman -S sdl2

    make

## Use

    Command line:
        ./paperview FOLDER SPEED

    For example:
        ./paperview train 5

    A lower SPEED number will result in a faster frame rate.
