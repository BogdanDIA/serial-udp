#!/bin/ash

PARAMS=""
if [[ -n "$BTATTACH_DEVICE" ]]; then
  PARAMS="$PARAMS -B $BTATTACH_DEVICE"
else
  echo no serial device provided, exiting... | tee -a charging-log.txt
  exit -1
fi

if [[ -n "$BTATTACH_BAUD" ]]; then
  PARAMS="$PARAMS -S $BTATTACH_BAUD"
else
  echo no baud for device provided, exiting... | tee -a charging-log.txt
  exit -1
fi

if [[ -n "$BTATTACH_PROTOCOL" ]]; then
  PARAMS="$PARAMS -P $BTATTACH_PROTOCOL"
else
  echo no protocol provided, exiting... | tee -a charging-log.txt
  exit -1
fi

if [[ "$BTATTACH_HWFLOW" -eq 0 ]]; then
  PARAMS="$PARAMS -N"
fi

echo "`date` set-btattach:btattach" | tee -a charging-log.txt

while [[ true ]]; do
  btattach $PARAMS 2>&1
  echo "set-btattach:btattach crashed with $?.  Respawning.." 2>&1 | tee -a charging-log.txt
  sleep 2
done

echo "`date` set-btattach:btattach fail exit"
