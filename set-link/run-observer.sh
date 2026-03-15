#!/bin/ash

CPATH=$(dirname "$0")/serial-udp.conf
. $CPATH
. ${SCRIPTS_PATH}/log-def.sh

DOWN_COUNT=0
DOWN_COUNT_MAX=6
res=""

while :; do
  HCINUM=$(hciconfig | grep hci | wc -l)
  HCINUM=$(($HCINUM-1))                 
  HCINUMB=$(bluetoothctl --timeout 0 list | grep Controller | wc -l)
  HCINUMB=$(($HCINUMB-1))
                        
  #kill btattach after a number or tries                                                                  
  if [[ $HCINUM != $HCINUMB ]]; then
    #res=$(hciconfig hci${HCINUM} | grep DOWN)
    #if [[ ! -z "$res" ]]; then
    if [[ true ]]; then
      log "Trying hciconfig up: hci$HCINUM"      
      hciconfig hci${HCINUM} up        
      DOWN_COUNT=$(($DOWN_COUNT+1))    
      log "DOWN_COUNT: $DOWN_COUNT"
      if [[ $DOWN_COUNT -ge $DOWN_COUNT_MAX ]]; then
        log "killing btattach"    
        killall -SIGKILL btattach
        killall -SIGKILL sudp-forwarder
        DOWN_COUNT=0
      fi
    else
      DOWN_COUNT=0
    fi
  fi
  sleep 20
done
