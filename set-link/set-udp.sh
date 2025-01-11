#!/bin/ash

CPATH=$(dirname "$0")/serial-udp.conf
. $CPATH
. ${SCRIPTS_PATH}/log-def.sh


PARAMS=""
if [[ -n "$SUDPFWD_DEVICE" ]]; then
  PARAMS="$PARAMS -d $SUDPFWD_DEVICE"
else
  log "no serial device provided, exiting..."
  exit -1
fi

if [[ -n "$SUDPFWD_BAUD" ]]; then
  PARAMS="$PARAMS -b $SUDPFWD_BAUD"
else
  log "no baud for device provided, exiting..."
  exit -1
fi

if [[ -n "$SUDPFWD_SERVER" ]]; then
  PARAMS="$PARAMS -s $SUDPFWD_SERVER"
else
  log "no server provided, exiting..."
  exit -1
fi

if [[ -n "$SUDPFWD_PORT" ]]; then
  PARAMS="$PARAMS -p $SUDPFWD_PORT"
else
  log "no port provided, exiting..."
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

log "`date` set-udp:sudp-forwarder"

while [[ true ]]; do
  /app/sudp-forwarder $PARAMS 2>&1 | tee -a /app/charging-log.txt
  log "set-udp:sudp-forwarder crashed with $?.  Respawning.."
  sleep 2
done

log "`date` set-udp:sudp-forwarder fail exit"

