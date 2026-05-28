basil-agent: main.cpp
	g++ -s -o $@ $^ -static-libgcc -static-libstdc++ -Wl,-Bstatic -lboost_log_setup -lboost_log -lboost_system -lboost_thread -lboost_program_options 

clean:
	rm -v basil-agent
