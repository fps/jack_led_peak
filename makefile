jack_led_peak: jack_led_peak.cc
	g++ -O3 $(CXXFLAGS) -Wall -o jack_led_peak jack_led_peak.cc -lgpiod -ljack -lboost_program_options
