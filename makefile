jack_peak_led: jack_peak_led.cc
	g++ -O3 $(CXXFLAGS) -Wall -o jack_peak_led jack_peak_led.cc -lgpiod -ljack -lboost_program_options
