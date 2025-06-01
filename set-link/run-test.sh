CPATH=$(dirname "$0")/serial-udp.conf
. $CPATH
. ${SCRIPTS_PATH}/log-def.sh

${BIN_PATH}/tests/sudp-test-a -d $BTATTACH_DEVICE -L 256 -M 2000

