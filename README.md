# GUARD
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
## Usage of GUARD tool
```

guard --help
GUARD Tool
Usage: guard [OPTIONS]

Options:
  -h,--help                Guard CLI tool options
  -c,--create TEXT         Create Guard record, expects physical path as input
  -i,--invalidate TEXT     Invalidate a single Guard record, expects physical
                           path as input
  -I,--invalidate-all      Invalidates all the Guard records
  -l,--list                List all the GUARD'ed resources
  -r,--reset               Erase all the Guard records
  -v,--version             Version of GUARD tool

```
**Note:** Physical path can be fetched from device tree, using ATTR_PHYS_DEV_PATH
attribute of the corresponding target.
Physical path formats supported by guard tool:-

* physical:sys-0/node-0/proc-0/mc-0/mi-0/mcc-0
* /sys-0/node-0/proc-0/mc-0/mi-0/mcc-0
* sys-0/node-0/proc-0/mc-0/mi-0/mcc-0

### Examples

* To create a guard record.
```
guard -c sys-0/node-0/proc-0/mc-0/mi-0/mcc-0
Success
```
* To list the guard records present in a system.
```
guard -l
ID       | ERROR    |  Type  | Path 
00000001 | 00000000 | manual | physical:sys-0/node-0/proc-0/mc-0/mi-0/mcc-0
```
* To erase all the guard records
```
guard -r
```
* To invalidate a single guard record
```
guard -i sys-0/node-0/proc-0/mc-0/mi-0/mcc-0
```
* To invalidate all the guard records
```
guard -I
```
