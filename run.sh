#!/bin/ash -e

. /app/libproduct.sh

log_info "Entering main loop..."

btattach -B /dev/serial/by-path/platforms-fd500000.pcie-pci-0000:01:00.0-usbv2-0:1.2.5:1.0-port0 -P h4 -S 921600 -N &

./sudp-forwarder -d /dev/serial/by-path/platform-fd500000.pcie-pci-0000:01:00.0-usbv2-0:1.2.6:1.0-port0 -b 921600 -N 

while :; do
  log_info "This is ESP32 HCI Proxy"
  sleep 30 
done
