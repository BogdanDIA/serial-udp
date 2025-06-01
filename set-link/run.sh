#!/bin/bash

CPATH=$(dirname "$0")/serial-udp.conf
. $CPATH
. ${SCRIPTS_PATH}/log-def.sh

log "Entering main loop..."

if [[ $DEBUG_TESTS == "false" ]]; then
  log "Starting btattach"
  ${SCRIPTS_PATH}/set-btattach.sh &

  log "Starting sudpforwarder"
  ${SCRIPTS_PATH}/set-udp.sh &
else
  log "debug_tests mode enabled"
  log "Starting sudpforwarder"
  ${SCRIPTS_PATH}/set-udp.sh &
fi

while :; do
  log "serial ports: $(ls -al /dev/serial/by-path/)"
  log "\n$(hciconfig)"
  sleep 60
done

