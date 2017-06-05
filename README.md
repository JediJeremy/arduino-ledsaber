# arduino-ledsaber
LED Saber Code - AndiG release

This is the latest firmware for my LED Saber project, documented here:

https://www.allaboutcircuits.com/projects/how-to-build-the-brightest-led-saber-in-the-world/

AndiG version features:
----------
This version is greatly improved from the original code:
* Button "click" to ignite.
* Long-press to retract blade.
* Settings stored to flash, so the saber remembers it's configuration. (rather than resetting on power-cycle)
* Added "echo" to the swing audio effect, and a new tunable option
* Added a brightness option
* rearranged the blade menu and made it more visible.
* Decreased "screesaver" idle timeout to 30 seconds.

Blade Menu:
----------
Counting from the hilt, the following options can be changed on the fly. Options are saved to flash when the blade is retracted.
* White : extension speed (also ignite and retract)
* White : sound volume
* Purple : presets (eight pre-programmed sets of options)
* Green : blade brightness
* Blue : blade hue
* Blue : blade saturation
* Yellow : buzz frequency
* Orange : hum1 frequency
* Orange : hum2 frequency
* Red : hum doppler shift
* Red : hum echo decay
* Black : no action (so accidentally bumping the knob does nothing)

Bugfixes:
----------
I found a major bug in the original code; the accellerometer wasn't being initialized correctly (just the gyro) 
and on SOME boards that was leading to a strange-sounding "bouncing" noise if the sensitivity was randomly set at maximum. 
(also, this was preventing the "screensaver" from activating)

The following code was added to the MPU6050_start() routine in mpu6050.h:
~~~~
  write_packet(I2C_MPU6050, 0x1C, 0b11101000); // Accel Configuration AFS_SEL = 1
  delay(20);
~~~~


Why is it called the "AndiG" version? This version was shipped in a specific set of sabers. To be revealed later, perhaps.
