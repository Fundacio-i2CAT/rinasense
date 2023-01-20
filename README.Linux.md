Linux build documentation
====

# 1. Creating the Tap device

A TAP device is a virtual network interface that allows applications
to receive ethernet packets from the network. It is a layer 2 virtual
network interface, as opposed to a TUN device, which is a layer 3
interface and only deals in IP packets.

## 1.1 Make your LAN network interface into a bridge

This step may vary depending on the variant of Linux you're using. I
generally have my LAN network interface as a bridge because it is
useful to connect virtual machines to my LAN

    # This file describes the network interfaces available on your system
    # and how to activate them. For more information, see interfaces(5).

    source /etc/network/interfaces.d/*

    # The loopback network interface
    auto lo
    iface lo inet loopback

    iface eth0 inet manual
    iface eth1 inet manual

    # LAN Bridge
    auto lan
    iface lan inet dhcp
        bridge_ports eth1

In this configuration, the main network interface my computer uses
talks to becomes `lan` instead of being `eth0` or `eth1`. It is
effectively a creates a virtual network switch with a single cable
connected (here `eth1`).

## 1.2 Create the TAP device

The following commands create a virtual TAP device called `rina00`.

    sudo ip tuntap add dev rina00 mode tap
    sudo ip link set rina00 up

## 1.3 Add the TAP device to your bridge

I'm still using the legacy `brctl` command to manipulate interfaces
that are part of a bridge.

    sudo brctl addif lan rina00

# 2. Building the Linux demo app

The main application that is used to test the Linux build is in
`Apps/linux-basic`.

The command line you can use to build the `linux-basic` app is the
following

    > mkdir build
    > cd build
    > cmake  ../../../CMakeLists.txt -DTARGET_TYPE=linux
    > make

# 3. Unit test

The Linux target is ingrated with the CMake test framework. `make
test` will run all the test programs that were generated durign the
build.

