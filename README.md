v4l2-loop - a kernel module to create V4L2 loop devices
==============================================================

This module does actually the same job as v4l2loopback but is has three main extensions:
- it supports multi-planar frame formats (mplane)
- it supports userptr buffers
- it supports dma-buf buffers

On the other hand, some of the functionality present in the v4l2loopback
is missing in this 4l2-loop project. This is:
- support in sysfs
- support for v4l2 controls

# BUILD
To build this module, just run:

    $ make

This should give you a file named "v4l2-loop.ko", which is the requested kernel module.
Such generated kernel module can be loaded and run only on the same kernel version
as the one used for compilation. So if you've updated your kernel, either explicitely
or implicilely via some "update manager", you would have to rebuild your module one more time,
this time against the updated version of your kernel (and kernel headers).

## BUILD AGAIN
To rebuild v4l2-loop.ko module, you have to first clean the previous artefacts of your build process.
To do so, just type

    $ make clean

# INSTALL
To install the module, run "make install" (you might have to be 'root' to have
all necessary permissions to install the module).

If your system has "sudo", do:

    $ make
    $ sudo make install
    $ sudo depmod -a

If your system lacks "sudo", do:

    $ make
    $ su
    (enter root password)
    $ make install
    $ depmod -a
    $ exit


(The `depmod -a` call will re-calculate module dependencies, in order to
automatically load additional kernel modules required by v4l2-loop.ko.

# RUN
Load the v4l2-loop module as root. Yet again.

If your system has "sudo", type:

    $ sudo modprobe v4l2-loop

If your system lacks "sudo", do:

    $ make
    $ su
    (enter root password)
    $ modprobe v4l2-loop
    $ exit

You can check which v4l2-loop devices are created by listing contents of
`/sys/devices/virtual/video4linux` directory. For example:

    $ ls /sys/devices/virtual/video4linux
    video4

These devices are ready to accept contents to show.

Tested producers:
- GStreamer-1.0: using the "v4l2sink" element

Example GStreamer pipelines could be:

    $ gst-launch-1.0 -v videotestsrc pattern=ball ! video/x-raw,width=640,height=480,framerate=2/1,format=GRAY8 ! v4l2sink device=/dev/video4

    $ gst-launch-1.0 -v videotestsrc pattern=smpte100 ! video/x-raw,width=640,height=480,framerate=2/1,format=RGB ! v4l2sink device=/dev/video4

    $ gst-launch-1.0 -v videotestsrc pattern=smpte75 ! video/x-raw,width=640,height=480,framerate=5/1,format=YUY2 ! v4l2sink device=/dev/video4

    $ gst-launch-1.0 -v videotestsrc pattern=smpte100 ! video/x-raw,width=640,height=480,framerate=2/1,format=NV16 ! v4l2sink device=/dev/video4

# OPTIONS

## debug
You may specify verbosity of debug messages emmited by v4l2-loop module. Default value is 0, which means that no
debug messaged will be printed. Max value is 3, enabling highest verbosity level. Thus typing

    $ sudo modprobe v4l2-loop debug=3

will turn on highest verbosity, whereas typing

    $ sudo modprobe v4l2-loop

will use default level (none debug messages will be emited).

## devices
You may specify how many virtual video loop devices will be created by this v4l2-loop module.
Default value is 1. So running

    $ sudo modprobe v4l2-loop devices=3

will create 3 virtual video devices. For example `/dev/video4`, `/dev/video5`, `/dev/video6`.
There is no functionality to create devices with arbitrary ids, specified during loading.
Ids will all the time be selected by underlying v4l2 framework, and that will be first available ids.

## buffers
This option allows to change the default minimum number of buffers needed before start streaming can be called.
This default value is 2 and if more buffers are needed just type during the loading for example

    $ sudo modprobe v4l2-loop buffers=8

## mplane
You may also specify whether this module works with single planar formats and buffers only or multiplanar ones.
Multiplanar mode is wider. In multiplanar mode all formats which are normally single planar are exported as
multiplanar formats with just one plane. Naturally multiplanar mode exports also formats with more than one plane,
which is not possible in single planar mode. By default, single planar mode is enabled.
To activate multiplanar mode, type

    $ sudo modprobe v4l2-loop mplane=1

during loading.
