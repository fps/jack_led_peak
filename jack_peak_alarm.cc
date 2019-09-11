#include <iostream>
#include <vector>
#include <gpiod.hpp>
#include <unistd.h>
#include <boost/program_options.hpp>
#include <jack/jack.h>

namespace po = boost::program_options;

jack_client_t *jack_client;
std::vector <jack_port_t*> jack_ports;

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
        ("green-led-threshold-dbfs", po::value<float>(&green_led_threshold_dbfs)->default_value(-6.0), "The threshold for the red LED")
        ("red-led-threshold-dbfs", po::value<float>(&red_led_threshold_dbfs)->default_value(-18.0), "The threshold for the red LED")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);    

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    gpiod::chip chip("/dev/gpiochip0", gpiod::chip::OPEN_BY_PATH);

    gpiod::line line = chip.get_line(18);
    gpiod::line_request line_request;
    line_request.consumer = "jpa";
    line_request.request_type = gpiod::line_request::DIRECTION_OUTPUT;
    line.request(line_request);
    line.set_value(1);
    sleep(1);
    

    return 0;
}
