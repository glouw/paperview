# PAPERVIEW 

A high performance animated desktop background setter for X11 that won't set your CPU on fire,
drain your laptop battery, or lower ya vidya game FPS.

![](screenshot.png)

Now imagine the above desktop background, moving, just list this: https://www.youtube.com/watch?v=6ZTiA885bWM

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
        ./paperview scenes/castle 5

    A lower SPEED number will result in a faster frame rate.

    Only BITMAP files are supported.

## Disclaimer

The art images included here I do not own.
They are for educational purposes only to demo the workings of paperview.
