#!/bin/ash

CPATH=$(dirname "$0")/serial-udp.conf
. $CPATH
. ${SCRIPTS_PATH}/log-def.sh

DOWN_COUNT=0
DOWN_COUNT_MAX=6

while :; do
  #check if last added controller is having proper number
  HCICOUNT=$(hciconfig | grep hci | wc -l)
  if [[ "$HCICOUNT" -ge 2 ]]; then
    CTRL=$(hciconfig | sed -n -E -e 's/^hci([0-9]+).*/\1/p')
    RES=$(echo $CTRL | awk '{ if ($1 != ($2+1)) {print "1"} else {print "0"} }')
    if [[ "$RES" != 0 ]]; then
      killall -SIGKILL btattach
    fi
  fi

  #find HCINUM with hciconfig
  HCINUM_H=$(hciconfig | grep hci | wc -l)
  HCINUM_H=$(($HCINUM_H-1)) 

  #find HCINUM with bluetoothctl
  HCINUM_B=$(bluetoothctl --timeout 0 list | grep Controller | wc -l)
  HCINUM_B=$(($HCINUM_B-1))

  log "HCICNUM_H: $HCINUM_H, HCINUM_B: $HCINUM_B"
                        
  #kill links after a number or tries                                                                  
  if [[ "$HCINUM_H" != "$HCINUM_B" ]]; then
    DOWN_COUNT=$(($DOWN_COUNT+1))    
    log "DOWN_COUNT: $DOWN_COUNT"
    if [[ $DOWN_COUNT -ge $DOWN_COUNT_MAX ]]; then
      log "killing links..."    
      killall -SIGKILL btattach
      killall -SIGKILL sudp-forwarder
      DOWN_COUNT=0
    fi
  else
    DOWN_COUNT=0
  fi
  sleep 20
done
