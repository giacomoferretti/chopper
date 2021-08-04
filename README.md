# chopper - Channel Hopper
chopper is a channel hopper written in C.

chopper uses `nl80211` to change the channel of the interface.

## Requisites
* libnl
  * Fedora / RHEL / CentOS: `sudo dnf install libnl3-devel`
  * Ubuntu / Debian: `sudo apt-get install libnl-genl-3-dev`

## Build
```shell
mkdir build
cd build
cmake ..
make
```

## Usage
```text
Usage: chopper [OPTION [ARG]] ...

Options:
  -h, --help                 show this help message
  -V, --version              show version
  -i, --interface <string>   interface name (must be in monitor mode)
  -c, --channels <list>      comma-separated list of channels
                             (default: 1,8,2,9,3,10,4,11,5,12,6,13,7)
  -d, --delay <int>          delay between each hop (default: 200)
  -t, --timeout <int>        exit the program after X seconds (default: 0)
```

## Other languages
Go: https://github.com/giacomoferretti/chopper-go