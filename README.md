# jack_led_peak

<pre>
A small jack utility (primary for the raspberry pi) to drive LEDs based on the peak values of the inputs
Allowed options:
  -h [ --help ]                         produce help message
  -a [ --jack-client-name ] arg (=jack_peak_alarm)
                                        The jack client name to use
  -e [ --jack-server-name ] arg (=default)
                                        The jack server name to use
  -n [ --jack-number-of-input-ports ] arg (=2)
                                        The number of input ports to watch
  -g [ --gpiod-green-led-offset ] arg (=23)
                                        The libgpiod line offset to use for the
                                        green indicator LED
  -r [ --gpiod-red-led-offset ] arg (=18)
                                        The libgpiod line offset to use for the
                                        red indicator LED
  -t [ --green-led-threshold-dbfs ] arg (=-18)
                                        The threshold for the red LED
  -u [ --red-led-threshold-dbfs ] arg (=-6)
                                        The threshold for the red LED
  -y [ --red-led-hysteresis-secs ] arg (=0.5)
                                        Approximate time for the red LED to go 
                                        off after being triggered
</pre>
