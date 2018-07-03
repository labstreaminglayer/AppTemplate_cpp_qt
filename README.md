# BestPracticesGUI App

This app doesn't send any useful data, but provides a starting point to develop
your own apps. You can copy the `appskeleton` folder to start a new app and
find annotated versions in the `doc` folder.

## Build system setup

The build configuration is in [`CMakeLists.txt`](doc/CMakeLists.txt.md).

You (and other users) can use this to build your app or create an IDE project
to develop the app.

## Sources

- [`main.cpp`](doc/main.cpp.md) is the entry point
- [`mainwindow.cpp`](doc/mainwindow.cpp.md) contains the UI code and code to
  access the recording device
- [`mainwindow.h`](doc/mainwindow.h.md) is the corresponding header
- [`sophisticated_recording_device.cpp`](doc/sophisticated_recording_device.cpp.md)
  and [`sophisticated_recording_device.h`](doc/sophisticated_recording_device.h.md)
  contain the code to connect to the device, check its status and acquire data

## License

Since using this app as a starting point is actively encouraged, it is licensed
under the [MIT](https://choosealicense.com/licenses/mit/) license.

If you want others to share the code to derivatives of your app, you should
consider licensing your app under a less permissive license like the
[MPL](https://choosealicense.com/licenses/mpl-2.0/) or
[GPL](https://choosealicense.com/licenses/gpl-3.0/).

Even in case you want to keep the MIT license, change the `LICENSE.txt` to
reflect that you're the copyright holder.
