#include "global.hpp"

int main(int argc, char **argv) {
	g_data::init();
	g_data::set_log_level(TRACE_LOG_LEVEL);

	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}
