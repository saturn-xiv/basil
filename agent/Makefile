export BASIL_BUILD_TIME := $(shell date -u)
export BASIL_VERSION := $(shell git describe --tags --always --dirty --first-parent)

basil-agent: main.cpp
	g++ -s -std=c++20 -static-libgcc -static-libstdc++ -o $@ $^ \
		-DBASIL_BUILD_TIME="\"$$BASIL_BUILD_TIME\"" -DBASIL_VERSION="\"$$BASIL_VERSION\"" \
		-Wl,-Bstatic \
		-lboost_log_setup -lboost_log -lboost_log_setup -lboost_system -lboost_thread -lboost_program_options -lboost_json \
		`pkg-config --libs curlpp` -lnghttp2 -lssh \
		-Wl,-Bdynamic `pkg-config --libs --static libcurl`
		

clean:
	rm -v basil-agent
