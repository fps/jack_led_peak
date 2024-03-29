#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <gpiod.h>
#include <unistd.h>
#include <boost/program_options.hpp>
#include <jack/jack.h>
#include <signal.h>

namespace po = boost::program_options;

jack_client_t *jack_client;
std::vector <jack_port_t*> jack_ports;

gpiod_line *green_led_line;
gpiod_line *red_led_line;

float green_led_threshold_gain;
float red_led_threshold_gain;
float red_led_blink_threshold_gain;
float red_led_blink_frequency_hz;
float red_led_hysteresis_secs;
float green_led_falloff_time_constant_secs;
float red_led_falloff_time_constant_secs;

float time_since_red_led_triggered = 0;
float time_since_red_led_blink_triggered = 0;

float blink_cycle = 0.0;
// float pwm_cycle = 0.0;

float green_old_max = 0.0;
float red_old_max = 0.0;

bool red_led_do_hold = false;
bool red_led_do_blink = false;

bool quit = false;

extern "C" 
{
    void signal_handler(int signo)
    {
        quit = true;
    }
}

int
process (jack_nframes_t nframes, void *arg)
{
    const float samplerate = (float)jack_get_sample_rate(jack_client);

    const float time_passed = (float)nframes / samplerate;

    float current_max = 0;
    for (unsigned int index = 0; index < jack_ports.size(); ++index)
    {
        jack_default_audio_sample_t *buffer;
        buffer = (jack_default_audio_sample_t*)jack_port_get_buffer(jack_ports[index], nframes);
        const float new_max = *std::max_element(buffer, buffer+nframes);
        const float new_min = *std::min_element(buffer, buffer+nframes);
        const float tmp = std::max(fabs(new_min), new_max);
        if (tmp > current_max) current_max = tmp;
    }

    if (current_max > green_old_max) green_old_max = current_max;
    if (current_max > red_old_max) red_old_max = current_max;

    green_old_max *= powf(0.5, time_passed / green_led_falloff_time_constant_secs);
    red_old_max *= powf(0.5, time_passed / red_led_falloff_time_constant_secs);

    if (red_old_max > red_led_threshold_gain)
    {
        time_since_red_led_triggered = 0;
    }

    if (red_old_max > red_led_blink_threshold_gain)
    {
        time_since_red_led_blink_triggered = 0;
        // blink_cycle = 0;
    }

#if 0
    if (time_since_red_led_blink_triggered < red_led_hysteresis_secs)
    {
        red_led_do_blink = true;
#if 0
        if (blink_cycle < 0.5)
        {
            gpiod_line_set_value(red_led_line, 1);
        }
        else
        {
            gpiod_line_set_value(red_led_line, 0);
        }
#endif
    }
    else
    {
        if (time_since_red_led_triggered < red_led_hysteresis_secs)
        {
            red_led_do_hold = true;
        }
#if 0
        if (time_since_red_led_triggered < red_led_hysteresis_secs || draw < (red_old_max - green_led_threshold_gain) / (red_led_threshold_gain - green_led_threshold_gain))
        {
            gpiod_line_set_value(red_led_line, 1);
        }
        else
        {
            gpiod_line_set_value(red_led_line, 0);
        }
#endif
    }

#if 0 
    if (green_old_max > green_led_threshold_gain || draw < (green_old_max / green_led_threshold_gain))
    {
        gpiod_line_set_value(green_led_line, 1);
    }
    else
    {
        gpiod_line_set_value(green_led_line, 0);
    }
#endif
#endif

    time_since_red_led_triggered += time_passed;
    time_since_red_led_blink_triggered += time_passed;

/*
    pwm_cycle += 20.0 * time_passed;
    pwm_cycle = fmodf(pwm_cycle, 1.0);
*/
    blink_cycle += red_led_blink_frequency_hz * time_passed;
    blink_cycle = fmodf(blink_cycle, 1.0);

    return 0;      
}

int main(int ac, char *av[])
{
    std::string jack_client_name;
    std::string jack_server_name;
    int jack_number_of_input_ports;

    std::string gpiod_chip_device_path;
    int gpiod_green_led_offset;
    int gpiod_red_led_offset;
    float green_led_threshold_dbfs;
    float red_led_threshold_dbfs;
    float red_led_blink_threshold_dbfs;
    float pwm_frequency_hz;

    std::vector<std::string> gpiod_led_offsets_string;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("jack-client-name,a", po::value<std::string>(&jack_client_name)->default_value("jack_led_peak"), "The jack client name to use")
        ("jack-server-name,e", po::value<std::string>(&jack_server_name)->default_value("default"), "The jack server name to use")
        ("jack-number-of-input-ports,n", po::value<int>(&jack_number_of_input_ports)->default_value(2), "The number of input ports to watch")
        ("gpiod-chip-device-path,d", po::value<std::string>(&gpiod_chip_device_path)->default_value("/dev/gpiochip0"), "The path of the gpiochip device to use")
        // ("gpiod-chip-led-offsets,o", po::value<std::vector<std::string>>(&gpiod_led_offsets_string)->default_value({"23,18"}), "Comma-separated list of gpiod line offsets to specify green,red LEDs")
        ("gpiod-green-led-offset,g", po::value<int>(&gpiod_green_led_offset)->default_value(23), "The libgpiod line offset to use for the green indicator LED")
        ("gpiod-red-led-offset,r", po::value<int>(&gpiod_red_led_offset)->default_value(18), "The libgpiod line offset to use for the red indicator LED")
        ("green-led-threshold-dbfs,t", po::value<float>(&green_led_threshold_dbfs)->default_value(-18.0), "The full saturation threshold for the green LED (note: this is not using oversampling, thus the value will be underestimated)")
        ("red-led-threshold-dbfs,u", po::value<float>(&red_led_threshold_dbfs)->default_value(-6.0), "The full saturation threshold for the red LED (note: this is not using oversampling, thus the value will be underestimated)")
        ("red-led-hysteresis-secs,y", po::value<float>(&red_led_hysteresis_secs)->default_value(1), "Approximate time for the red LED to stay on after reaching full saturarion")
        ("red-led-blink-threshold-dbfs,b", po::value<float>(&red_led_blink_threshold_dbfs)->default_value(-1.0), "The threshold at which the red LED starts to blink")
        ("red-led-blink-frequency-hz,f", po::value<float>(&red_led_blink_frequency_hz)->default_value(5), "The red LED blinking frequency (approximate)")
        ("green-led-falloff-time-constant-secs,z", po::value<float>(&green_led_falloff_time_constant_secs)->default_value(0.1), "The green LED's time for the exponential falloff to drop to half the peak value")
        ("red-led-falloff-time-constant-secs,x", po::value<float>(&red_led_falloff_time_constant_secs)->default_value(0.4), "The red LED's time for the exponential falloff to drop to half the peak value")
        ("pwm-frequency_hz,p", po::value<float>(&pwm_frequency_hz)->default_value(10000), "The frequency with which to perform PWM (approximate)")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);    

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    green_led_threshold_gain = powf(10.0, green_led_threshold_dbfs/20.0);
    red_led_threshold_gain = powf(10.0, red_led_threshold_dbfs/20.0);
    red_led_blink_threshold_gain = powf(10.0, red_led_blink_threshold_dbfs/20.0);

    gpiod_chip *chip;
    chip = gpiod_chip_open(gpiod_chip_device_path.c_str());
    if (NULL == chip)
    {
        std::cout << "Failed to open chip" << std::endl;
        return 1;
    }

    green_led_line = gpiod_chip_get_line(chip, gpiod_green_led_offset);
    red_led_line = gpiod_chip_get_line(chip, gpiod_red_led_offset);

    gpiod_line_request_config line_request;
    line_request.consumer = "jack_led_peak";
    line_request.request_type = GPIOD_LINE_REQUEST_DIRECTION_OUTPUT;

    gpiod_line_request(green_led_line, &line_request, 0);
    gpiod_line_request(red_led_line, &line_request, 0);

    jack_status_t jack_status;
    jack_client = jack_client_open(jack_client_name.c_str(), JackNullOption, &jack_status, jack_server_name.c_str());

    if (NULL == jack_client) {
        std::cout << "Failed to create jack_client. Exiting." << std::endl;
        return 1;
    }

    jack_set_process_callback(jack_client, process, 0);

    for (int index = 0; index < jack_number_of_input_ports; ++index)
    {
        std::stringstream name_stream;
        name_stream << "input";
        name_stream << index;
        jack_ports.push_back(jack_port_register(jack_client, name_stream.str().c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0));
        if (jack_ports[index] == NULL) {
            std::cout << "Failed to register port with index " << index << std::endl;
            return 1;
        }
    }

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    if (jack_activate(jack_client))
    {
        std::cout << "Failed to activate jack_client" << std::endl;
        return 1;
    }

    const unsigned int pwm_usecs = (unsigned int)(1000000 * 1.0  / pwm_frequency_hz);

    while(false == quit)
    {
        const float draw1 = (float)rand()/(float)RAND_MAX;
        const float draw2 = (float)rand()/(float)RAND_MAX;

        if (draw1 < (green_old_max / green_led_threshold_gain))
        {
            gpiod_line_set_value(green_led_line, 1);
        }
        else
        {
            gpiod_line_set_value(green_led_line, 0);
        }

        if (draw2 < ((red_old_max - green_led_threshold_gain) / (red_led_threshold_gain)))
        {
            gpiod_line_set_value(red_led_line, 1);
        }
        else
        {
            gpiod_line_set_value(red_led_line, 0);
        }

        if (time_since_red_led_triggered < red_led_hysteresis_secs)
        {
            gpiod_line_set_value(red_led_line, 1); 
        }

        if (time_since_red_led_blink_triggered < red_led_hysteresis_secs)
        {
            if (blink_cycle < 0.5)
                { gpiod_line_set_value(red_led_line, 1); }
            else
                { gpiod_line_set_value(red_led_line, 0); }
        }

        usleep(pwm_usecs);

        // TODO: measure time elapsed and use it instead of the nominal value
        blink_cycle += red_led_blink_frequency_hz * (float)pwm_usecs/1000000.0;
        blink_cycle = fmodf(blink_cycle, 1.0);
    }

    jack_deactivate(jack_client);

    std::cout << "Bye." << std::endl;

    return 0;
}
