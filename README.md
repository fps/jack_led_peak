# jack_led_peak

A small jack utility (primary for the raspberry pi) to drive LEDs based on the peak values of the inputs.

<pre>
jack_peak_led -h
Allowed options:
  -h [ --help ]                         produce help message
  -a [ --jack-client-name ] arg (=jack_peak_led)
                                        The jack client name to use
  -e [ --jack-server-name ] arg (=default)
                                        The jack server name to use
  -n [ --jack-number-of-input-ports ] arg (=2)
                                        The number of input ports to watch
  -d [ --gpiod-chip-device-path ] arg (=/dev/gpiochip0)
                                        The path of the gpiochip device to use
  -g [ --gpiod-green-led-offset ] arg (=23)
                                        The libgpiod line offset to use for the
                                        green indicator LED
  -r [ --gpiod-red-led-offset ] arg (=18)
                                        The libgpiod line offset to use for the
                                        red indicator LED
  -t [ --green-led-threshold-dbfs ] arg (=-18)
                                        The full saturation threshold for the 
                                        green LED (note: this is not using 
                                        oversampling, thus the value will be 
                                        underestimated)
  -u [ --red-led-threshold-dbfs ] arg (=-6)
                                        The full saturation threshold for the 
                                        red LED (note: this is not using 
                                        oversampling, thus the value will be 
                                        underestimated)
  -y [ --red-led-hysteresis-secs ] arg (=0.5)
                                        Approximate time for the red LED to 
                                        stay on after reaching full saturarion
  -b [ --red-led-blink-threshold-dbfs ] arg (=-1)
                                        The threshold at which the red LED 
                                        starts to blink
  -f [ --red-led-blink-frequency-hz ] arg (=5)
                                        The red LED blinking frequency 
                                        (approximate)
  -z [ --falloff-time-constant-secs ] arg (=0.100000001)
                                        The time for the exponential falloff to
                                        drop to half the peak value
