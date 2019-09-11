#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <gpiod.hpp>
#include <unistd.h>
#include <boost/program_options.hpp>
#include <jack/jack.h>

namespace po = boost::program_options;

jack_client_t *jack_client;
std::vector <jack_port_t*> jack_ports;

gpiod::line green_led_line;
gpiod::line red_led_line;

float green_led_threshold_gain;
float red_led_threshold_gain;
float red_led_hysteresis_secs;
float time_since_red_led_triggered = 0;

int
process (jack_nframes_t nframes, void *arg)
{
    float current_max = 0;
    for (int index = 0; index < jack_ports.size(); ++index)
    {
        jack_default_audio_sample_t *buffer;
        buffer = (jack_default_audio_sample_t*)jack_port_get_buffer(jack_ports[index], nframes);
        const float new_max = *std::max_element(buffer, buffer+nframes);
        const float new_min = *std::min_element(buffer, buffer+nframes);
        const float tmp = std::max(fabs(new_min), new_max);
        if (tmp > current_max) current_max = tmp;
    }

    if (current_max > red_led_threshold_gain)
    {
        time_since_red_led_triggered = 0;
    }

    if (time_since_red_led_triggered < red_led_hysteresis_secs)
    {
        red_led_line.set_value(1);
    }
    else
    {
        red_led_line.set_value(0);
    }

    if (current_max > green_led_threshold_gain)
    {
        green_led_line.set_value(1);
    }
    else
    {
        green_led_line.set_value(0);
    }

    time_since_red_led_triggered += (float)nframes / (float)jack_get_sample_rate(jack_client);

#if 0
    jack_default_audio_sample_t *in, *out;
    
    in = jack_port_get_buffer (input_port, nframes);
    out = jack_port_get_buffer (output_port, nframes);
    memcpy (out, in,
        sizeof (jack_default_audio_sample_t) * nframes);
#endif
    return 0;      
}

int main(int ac, char *av[])
{
    int gpiod_green_led_offset;
    int gpiod_red_led_offset;
    std::string jack_client_name;
    std::string jack_server_name;
    int jack_number_of_input_ports;

    float green_led_threshold_dbfs;
    float red_led_threshold_dbfs;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("jack-client-name,a", po::value<std::string>(&jack_client_name)->default_value("jack_peak_alarm"), "The jack client name to use")
        ("jack-server-name,e", po::value<std::string>(&jack_server_name)->default_value("default"), "The jack server name to use")
        ("jack-number-of-input-ports,n", po::value<int>(&jack_number_of_input_ports)->default_value(2), "The number of input ports to watch")
        ("gpiod-green-led-offset,g", po::value<int>(&gpiod_green_led_offset)->default_value(23), "The libgpiod line offset to use for the green indicator LED")
        ("gpiod-red-led-offset,r", po::value<int>(&gpiod_red_led_offset)->default_value(18), "The libgpiod line offset to use for the red indicator LED")
        ("green-led-threshold-dbfs,t", po::value<float>(&green_led_threshold_dbfs)->default_value(-18.0), "The threshold for the red LED")
        ("red-led-threshold-dbfs,u", po::value<float>(&red_led_threshold_dbfs)->default_value(-6.0), "The threshold for the red LED")
        ("red-led-hysteresis-secs,y", po::value<float>(&red_led_hysteresis_secs)->default_value(0.5), "Approximate time for the red LED to go off after being triggered")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);    

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    green_led_threshold_gain = powf(10.0, green_led_threshold_dbfs/10.0);
    red_led_threshold_gain = powf(10.0, red_led_threshold_dbfs/10.0);

    gpiod::chip chip("/dev/gpiochip0", gpiod::chip::OPEN_BY_PATH);

    green_led_line = chip.get_line(gpiod_green_led_offset);
    red_led_line = chip.get_line(gpiod_red_led_offset);

    gpiod::line_request line_request;
    line_request.consumer = "jpa";
    line_request.request_type = gpiod::line_request::DIRECTION_OUTPUT;

    green_led_line.request(line_request);
    red_led_line.request(line_request);

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

    if (jack_activate(jack_client))
    {
        std::cout << "Failed to activate jack_client" << std::endl;
        return 1;
    }

    sleep(-1);

    return 0;
}
