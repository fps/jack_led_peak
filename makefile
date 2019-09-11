jack_peak_led: jack_peak_led.cc
	g++ -O3 $(CXXFLAGS) -o jack_peak_led jack_peak_led.cc -lgpiodcxx -ljack -lboost_program_options
