#include "global.hpp"
#include "mqtt_agent.h"

void init_config(const char *file_name) {
	Config cfg;
	if (cfg.init(file_name) == false) {
		std::cout << "Config file path wrong. Check config file path." << std::endl;
		exit(-1);
	}

	mqtt_conn_info mq_info;
	mq_info.mqtt_address_ = cfg.str("MQTT", "ADDRESS", "");
	mq_info.mqtt_user_ = cfg.str("MQTT", "USERNAME", "");
	mq_info.mqtt_password_ = cfg.str("MQTT", "PASSWORD", "");

	g_data::log(INFO_LOG_LEVEL, "[main] MQTT address : %s", mq_info.mqtt_address_.c_str());
	g_data::log(INFO_LOG_LEVEL, "[main] MQTT username : %s", mq_info.mqtt_user_.c_str());
	g_data::log(INFO_LOG_LEVEL, "[main] MQTT password : %s", mq_info.mqtt_password_.c_str());

	g_data::set_mqtt_info(mq_info);
}

int main(int argc, char **argv) {
	g_data::init();
	g_data::set_log_level(TRACE_LOG_LEVEL);

	init_config("config/spi2mqtt.ini");

	mqtt_agent mq_agent_;

	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}
