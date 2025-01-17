#!/bin/ash

CPATH=$(dirname "$0")/serial-udp.conf
. $CPATH
. ${SCRIPTS_PATH}/log-def.sh

PARAMS=""
if [[ -n "$BTATTACH_DEVICE" ]]; then
  PARAMS="$PARAMS -B $BTATTACH_DEVICE"
else
  log "no serial device provided, exiting..."
  exit 1
fi

if [[ -n "$BTATTACH_BAUD" ]]; then
  PARAMS="$PARAMS -S $BTATTACH_BAUD"
else
  echo "no baud for device provided, exiting..."
  exit 1
fi

if [[ -n "$BTATTACH_PROTOCOL" ]]; then
  PARAMS="$PARAMS -P $BTATTACH_PROTOCOL"
else
  log "no protocol provided, exiting..."
  exit 1
fi

if [[ "$BTATTACH_HWFLOW" -eq 0 ]]; then
  PARAMS="$PARAMS -N"
fi

log "`date` set-btattach:btattach"

while [[ true ]]; do
  btattach $PARAMS 2>&1
  log "set-btattach:btattach crashed with $?.  Respawning.."
  sleep 2
done

log "`date` set-btattach:btattach fail exit"
