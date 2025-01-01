#!/bin/ash

PARAMS=""
if [[ -n "$SUDPFWD_DEVICE" ]]; then
  PARAMS="$PARAMS -d $SUDPFWD_DEVICE"
else
  echo no serial device provided, exiting... | tee -a charging-log.txt
  exit -1
fi

if [[ -n "$SUDPFWD_BAUD" ]]; then
  PARAMS="$PARAMS -b $SUDPFWD_BAUD"
else
  echo no baud for device provided, exiting... | tee -a charging-log.txt
  exit -1
fi

if [[ -n "$SUDPFWD_SERVER" ]]; then
  PARAMS="$PARAMS -s $SUDPFWD_SERVER"
else
  echo no server provided, exiting... | tee -a charging-log.txt
  exit -1
fi

if [[ -n "$SUDPFWD_PORT" ]]; then
  PARAMS="$PARAMS -p $SUDPFWD_PORT"
else
  echo no port provided, exiting... | tee -a charging-log.txt
  exit -1
fi

if [[ "$SUDPFWD_HWFLOW" -eq 0 ]]; then
  PARAMS="$PARAMS -N"
fi

if [[ "$SUDPFWD_FLOWDEBUG" -eq 0 ]]; then
  PARAMS="$PARAMS -F"
fi

if [[ "$SUDPFWD_DATADEBUG" -eq 0 ]]; then
  PARAMS="$PARAMS -D"
fi

echo "`date` set-udp:sudp-forwarder" | tee -a charging-log.txt

while [[ true ]]; do
  /app/sudp-forwarder $PARAMS 2>&1 | tee -a charging-log.txt
  echo "set-udp:sudp-forwarder crashed with $?.  Respawning.." 2>&1 | tee -a charging-log.txt
  sleep 2
done

echo "`date` set-udp:sudp-forwarder fail exit" | tee -a charging-log.txt

