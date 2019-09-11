jack_peak_alarm: jack_peak_alarm.cc
	g++ -o jack_peak_alarm jack_peak_alarm.cc -lgpiodcxx -ljack -lboost_program_options
