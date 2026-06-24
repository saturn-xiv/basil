export BASIL_BUILD_TIME := $(shell date -u)
export BASIL_VERSION := $(shell git describe --tags --always --dirty --first-parent)

basil-agent: main.cpp
	clang++ -s -std=c++20 -stdlib=libc++ -o $@ $^ \
		-DBASIL_BUILD_TIME="\"$$BASIL_BUILD_TIME\"" -DBASIL_VERSION="\"$$BASIL_VERSION\"" \
		-DBOOST_LOG_DYN_LINK -lboost_log_setup -lboost_log -lboost_log_setup -lboost_system -lboost_thread -lboost_program_options -lboost_json \
		`pkg-config --cflags --libs curlpp`

clean:
	rm -v basil-agent
