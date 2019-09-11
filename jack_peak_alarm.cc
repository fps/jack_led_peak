#include <gpiod.hpp>
#include <unistd.h>

#include <jack/jack.h>

int main()
{
	gpiod::chip chip("/dev/gpiochip0", gpiod::chip::OPEN_BY_PATH);

	gpiod::line line = chip.get_line(18);
	gpiod::line_request line_request;
	line_request.consumer = "jpa";
	line_request.request_type = gpiod::line_request::DIRECTION_OUTPUT;
	line.request(line_request);
	line.set_value(1);
	sleep(1);
	
}
