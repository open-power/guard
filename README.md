# guard
Guarding the system against failures by permanently isolating faulty units.
guard records contains the permanently isolated components details.
GUARD repository provides the libraries and tools to create/list/delete guard records
## Building
Need `meson` and `ninja`. Alternatively, source an OpenBMC ARM/x86 SDK.
```
meson build && ninja -C build
```
To use power system device for guard FRU's.
```
meson build -Ddevtree=enabled && ninja -C build
```
To build libguard with verbose level to get required trace.\
Supported verbose level:\
`0` - Emergency, `1` - Alert, `2` - Critical, `3` - Error, `4` - Warning,
`5` - Notice, `6` - Info, `7` - Debug\
By default verbose level is Error.
```
meson build -Dverbose=7 && ninja -C build
```
## To run unit tests
Tests can be run in the CI docker container, or with an OpenBMC x86 sdk(see
below for x86 steps).
```
meson -Doe-sdk=enabled -Dtests=enabled build
ninja -C build test
```
