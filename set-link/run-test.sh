CPATH=$(dirname "$0")/serial-udp.conf
. $CPATH
. ${SCRIPTS_PATH}/log-def.sh

${SCRIPTS_PATH}/tests/sudp-test-a -D $BTATTACH_DEVICE -L 256 -M 2000

