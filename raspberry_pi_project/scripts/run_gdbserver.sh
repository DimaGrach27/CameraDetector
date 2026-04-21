#!/usr/bin/env bash

APP_DIR="/home/pi/camera_tracking"
BIN_NAME="Camera_tracking"
PORT="2345"
LOG_FILE="/tmp/camera_tracking_gdbserver.log"

cd "$APP_DIR" || exit 1

OLD_PIDS=$(pgrep gdbserver || true)
if [ -n "$OLD_PIDS" ]; then
    echo "Stopping old gdbserver: $OLD_PIDS"
    kill $OLD_PIDS || true
    sleep 1
fi

export LD_LIBRARY_PATH="/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH:-}"

rm -f "$LOG_FILE"

nohup gdbserver ":$PORT" "./bin/$BIN_NAME" > "$LOG_FILE" 2>&1 &
sleep 2

if ss -ltn | grep -q ":$PORT"; then
    echo "gdbserver started on port $PORT"
    echo "log: $LOG_FILE"
else
    echo "gdbserver failed to start"
    echo "---- log ----"
    cat "$LOG_FILE" 2>/dev/null || true
    exit 1
fi