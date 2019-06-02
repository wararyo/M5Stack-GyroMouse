# M5Stack-GyroMouse
M5Stack mouse app using Bluetooth Low Energy

This is a M5Stack app which makes it gyro-sensor mouse via Bluetooth Low Energy MIDI.

## Note

Now this app has trouble with connecting function.  
When connecting more than twice, please delete pairing from your PC.

It uses 1.3M byte of program memory.  
If you use M5Stack 4MB-flash model, change the partition to "Huge App (3MB No OTA)".

Contributors welcome!

# How to Install

## Using SDUpdater(or LovyanLauncher) (recommended)

**This way cannot be used for 4MB-flash model**

1. Install [SDUpdater](https://github.com/tobozo/M5Stack-SD-Updater) or [LovyanLauncher](https://github.com/lovyan03/M5Stack_LovyanLauncher) into your M5Stack.
2. Download GyroMouse_0.1.0_bin.zip from [Releases](https://github.com/wararyo/M5Chorder/releases).
3. Prepare an SD Card and copy files like the following.
4. Insert the SD into the M5Stack and execute.

```
(SD Root)
/
├GyroMouse.bin
├json
│└GyroMouse.json
└jpg
  └GyroMouse_gh.jpg
```

## Using PlatformIO
1. Install Visual Studio Code.
2. Install PlatformIO.
3. Clone the repository.
4. Open repository folder in VSCode and run "PlatformIO: Upload".

## Using esptool (untested)
1. Install esptool.
2. Download GyroMouse_0.1.0_bin.zip from [Releases](https://github.com/wararyo/M5Chorder/releases).
3. Run the following.

``` sh
# Case of 16MB-flash model and of default 16MB partition
esptool --chip esp32 --port /dev/tty.SLAB_USBtoUART --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_freq 40m 0x10000 BLEChorder.bin
```

# License

MIT
