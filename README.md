# guard
Guard record definition and libraries/tools for manipulating them
=======
## To Build
Need `meson` and `ninja`. Alternatively, source an OpenBMC ARM/x86 SDK.
```
meson build && ninja -C build
```
## To run unit tests
Tests can be run in the CI docker container, or with an OpenBMC x86 sdk(see
below for x86 steps).
```
meson -Doe-sdk=enabled -Dtests=enabled build
ninja -C build test
```
