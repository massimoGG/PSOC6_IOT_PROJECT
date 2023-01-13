# Project

This psoc project allows pairing of new clients when pressing the USER button.

**Sequence of operation**

1. The PSOC broadcasts a Bluetooth Advertisement

2. The user's phone pairs to the PSOC.

3. The PSOC stores and publishes the info to the MQTT broker.

4. When the user's phone disconnects, the alarm system activates and publishes the system's state to the MQTT broker (`ACTIVE`), the LED turns on indicating that the alarm system is active.

5. The user can press the PSOC's button to disable the alarm.
