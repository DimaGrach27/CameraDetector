#!/bin/bash
exec /opt/homebrew/bin/aarch64-unknown-linux-gnu-gdb \
    -ex "set sysroot /Users/dhrachov/rpi-sysroot" \
#    -ex "set solib-search-path /Users/dhrachov/rpi-sysroot/lib:/Users/dhrachov/rpi-sysroot/usr/lib:/Users/dhrachov/rpi-sysroot/usr/lib/aarch64-linux-gnu:/Users/dhrachov/rpi-sysroot/opt" \
    "$@"