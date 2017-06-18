# `sylkie`

A command line tool and library for testing networks for common address
spoofing security vulnerabilities in IPv6 networks using the Neighbor
Discovery Protocol.

## Getting Started

### Dependencies

 - [libseccomp](https://github.com/seccomp/libseccomp)
 - [json-c](https://github.com/json-c/json-c)

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
`sylkie <subcommand> -h` for more details.

### DoS (Router Advert)

The basic usage of the `sylkie` router advert command is listed below.
This command will send a Router Advertisement message to the given ip
or the all nodes multicast addres with the Valid and Preferred
Lifetime set to 0 for the router ip and prefix provided.

```
sylkie ra -interface <interface> \
    --target-mac <mac of router> \
    --router-ip <ip of router> \
    --prefix <router prefix> \
    --timeout <time between adverts> \
    --repeat <number of times to send the request>
}
```

As you probably notice that is quite a bit of info, that can be annoying
to type out on the command line. `sylkie` also accepts json to aleviate
this. Were the subcommand (`ra`, `na`) is a key whos value is an array
of objects with the keys and values being the corresponding option
and value. See the examples for details.

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

As json this command would be

```
{
    "ra": [
        {
            "interface": "br0",
            "target-mac": "0c:c4:7a:6c:cd:54",
            "router-ip": "fe80::ec4:7aff:fe6c:cd54",
            "prefix": 64,
            "repeat": -1,
            "timeout": 10
        }
    ]
}
```

### Address spoofing (Neighbor Advert)

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

## FAQ

### Why write this?

Learning is fun. I didn't know much about The ND protocol so I read
[RFC 4861](https://tools.ietf.org/html/rfc4861) and this seemed like
a good way to learn by practice.
