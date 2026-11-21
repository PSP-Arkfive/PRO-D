#!/usr/bin/env bash
set -euo pipefail

root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
workdir=$(mktemp -d "${TMPDIR:-/tmp}/pro-build.XXXXXXXX")
trap 'rm -rf -- "$workdir"' EXIT

mkdir -p "$workdir/source" "$workdir/packages"
tar -C "$root" --exclude=./.git --exclude=./dist --exclude=./.usbdevice-build -cf - . |
    tar -C "$workdir/source" -xf -
cd "$workdir/source"

make libs_latest
cp .usbdevice-build/usbdevice.prx "$workdir/usbdevice.prx"

for firmware in 620 639 661; do
    echo "Building PRO for firmware $firmware"
    make clean "CONFIG_${firmware}=1"
    mkdir -p .usbdevice-build
    cp "$workdir/usbdevice.prx" .usbdevice-build/usbdevice.prx
    make -o libs_latest "CONFIG_${firmware}=1"
    test -s "dist/FLASH${firmware}.PRO"
    cp "dist/FLASH${firmware}.PRO" "$workdir/packages/"
done

installer=dist/PSP/GAME/PROINSTALLER
test -s "$installer/EBOOT.PBP"
test -s "$installer/usbdevice.prx"
mkdir -p "$root/dist/PSP/GAME/PROINSTALLER"
cp "$installer/EBOOT.PBP" "$installer/usbdevice.prx" "$root/dist/PSP/GAME/PROINSTALLER/"
for firmware in 620 639 661; do
    package="FLASH${firmware}.PRO"
    cp "$workdir/packages/$package" "$root/dist/$package"
    cp "$workdir/packages/$package" "$root/dist/PSP/GAME/PROINSTALLER/$package"
done
echo "PRO packages written to $root/dist"
