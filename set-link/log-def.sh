log() {
  echo "$1" | tee -a ${BIN_PATH}/charging-log.txt >&1
}
