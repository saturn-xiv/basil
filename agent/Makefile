basil-agent: main.cpp
	g++ -s -std=c++20 -o $@ $^ \
		-DBOOST_LOG_DYN_LINK -lboost_log_setup -lboost_log -lboost_log_setup -lboost_system -lboost_thread -lboost_program_options -lboost_json \
		`pkg-config --cflags --libs curlpp`

clean:
	rm -v basil-agent
