#!/usr/bin/env bash
set -euo pipefail

PI_HOST="${PI_HOST:-pi@192.168.0.107}"
PI_APP_DIR="${PI_APP_DIR:-/home/pi/camera_tracking}"
BUILD_DIR="${BUILD_DIR:-build/rpi-debug-local}"
BIN_NAME="${BIN_NAME:-Camera_tracking}"

echo "Creating target directories on Pi..."
ssh "$PI_HOST" "mkdir -p $PI_APP_DIR/bin $PI_APP_DIR/resources $PI_APP_DIR/models"

echo "Uploading binary..."
rsync -avz "$BUILD_DIR/$BIN_NAME" "$PI_HOST:$PI_APP_DIR/bin/"

echo "Uploading resources..."
rsync -avz resources/ "$PI_HOST:$PI_APP_DIR/resources/"

echo "Uploading scripts..."
rsync -avz scripts/run_gdbserver.sh "$PI_HOST:$PI_APP_DIR/scripts/run_gdbserver.sh"

echo "Deploy finished."