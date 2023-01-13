## Mosquitto Configuration
HOWTO: http://www.steves-internet-guide.com/mossquitto-conf-file/

## Certificate generation
TLS: http://www.steves-internet-guide.com/mosquitto-tls/

## Configuration file
See mosquitto.conf

## Run
`podman run -it -p 1883:1883 -p 9001:9001 -p 8883:8883 -v /opt/mosquitto/:/mosquitto eclipse-mosquitto`
