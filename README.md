# arduino-ledsaber
LED Saber Code - AndiG release

This is the latest firmware for my LED Saber project, documented here:

https://www.allaboutcircuits.com/projects/how-to-build-the-brightest-led-saber-in-the-world/

AndiG version features:
----------
This version is greatly improved from the original code:
* Long-press to ignite and retract blade.
* Settings stored to flash, so the saber remembers it's configuration. (rather than resetting on power-cycle)
* Added a brightness option, rearranged the blade menu slightly.
* Decreased "screesaver" idle timeout to 30 seconds.

Bugfixes:
----------
I found a major bug in the original code; the accellerometer wasn't being initialized correctly (just the gyro) 
and on SOME boards that was leading to a strange-sounding "bouncing" noise if the sensitivity was randomly set at maximum. 
(also, the shutdown "screensaver" wasn't kicking in)

The following code was added to the MPU6050_start() routine in mpu6050.h:
~~~~
  write_packet(I2C_MPU6050, 0x1C, 0b11101000); // Accel Configuration AFS_SEL = 1
  delay(20);
~~~~


Why is it called the "AndiG" version? To be revealed later, perhaps.
