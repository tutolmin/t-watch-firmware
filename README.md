# t-watch-firmware

Firmware for various T-Watches

### S3

[GitHub - Xinyuan-LilyGO/LilyGoLib-PlatformIO](https://github.com/Xinyuan-LilyGO/LilyGoLib-PlatformIO?tab=readme-ov-file)

[Xinyuan-LilyGO/LilyGoLib · GitHub Arduino](https://github.com/Xinyuan-LilyGO/LilyGoLib/blob/master/docs/lilygo-t-watch-s3.md)

---

### Stable release link

```
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

### Permission error on serial port

> Port monitor error: command 'open' failed: Permission denied. Could not connect to /dev/ttyACM0 serial port.

Add current user to the group and reboot

```
sudo usermod -a -G dialout $USER
```

or

```
curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules
sudo systemctl daemon-reload

```


