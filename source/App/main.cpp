#include "global.hpp"
#include "mqtt_agent.h"
#include "spi_slave.h"
#include <boost/algorithm/string.hpp>

void init_config(const char *file_name) {
	Config cfg;
	if (cfg.init(file_name) == false) {
		std::cout << "Config file path wrong. Check config file path." << std::endl;
		exit(-1);
	}

	mqtt_conn_info mq_info;
	mq_info.mqtt_address = cfg.str("MQTT", "ADDRESS", "");
	mq_info.mqtt_user = cfg.str("MQTT", "USERNAME", "");
	mq_info.mqtt_password = cfg.str("MQTT", "PASSWORD", "");
	mq_info.pub_topic = cfg.str("MQTT", "PUBLISH_TOPIC");
	mq_info.subs_topic = cfg.str("MQTT", "SUBSCRIBE_TOPIC");

	g_data::log(INFO_LOG_LEVEL, "[main] MQTT address : %s", mq_info.mqtt_address.c_str());
	g_data::log(INFO_LOG_LEVEL, "[main] MQTT username : %s", mq_info.mqtt_user.c_str());
	g_data::log(INFO_LOG_LEVEL, "[main] MQTT password : %s", mq_info.mqtt_password.c_str());
	g_data::log(INFO_LOG_LEVEL, "[main] MQTT publish topic : %s", mq_info.pub_topic.c_str());
	g_data::log(INFO_LOG_LEVEL, "[main] MQTT subscribe topic : %s", mq_info.subs_topic.c_str());
	g_data::set_mqtt_info(mq_info);

	spi_info spi;
	spi.device = cfg.str("SPI", "DEVICE", "/dev/spidev0.0");
	spi.speed = cfg.getiValue("SPI", "SPEED", 500000);
	g_data::set_spi_info(spi);
	g_data::log(INFO_LOG_LEVEL, "[main] SPI DEVICE : %s", spi.device.c_str());
	g_data::log(INFO_LOG_LEVEL, "[main] SPI SPEED : %d", spi.speed);

	int serial_count = cfg.getiValue("PIN_SERIAL", "COUNT", 0);
	if (serial_count <= 0) {
		std::cout << "PinSerial count is smaller than zero." << std::endl;
		exit(-1);
	}

	std::map<std::string, std::string> serial_map;
	for (int i = 0; i < serial_count; i++) {
		char idx[256] = {0, };
		snprintf(idx, sizeof(idx), "PIN%d", i + 1);

		std::string line = cfg.str("PIN_SERIAL", idx, "");
		std::vector<std::string> split;
		boost::algorithm::split(split, line, boost::algorithm::is_any_of(":"));

		if (split.size() != 2) {
			std::cout << "PinSerial value is wrong." << std::endl;
			exit(-1);
		}

		serial_map[split[0]] = split[1];
	}
	g_data::set_serial_map(serial_map);

	int type_count = cfg.getiValue("PIN_TYPE", "COUNT", 0);
	if (type_count <= 0) {
		std::cout << "PinType count is smaller than zero." << std::endl;
		exit(-1);
	}

	std::map<std::string, std::string> type_map;
	for (int i = 0; i < serial_count; i++) {
		char idx[256] = {0, };
		snprintf(idx, sizeof(idx), "PIN%d", i + 1);

		std::string line = cfg.str("PIN_TYPE", idx, "");
		std::vector<std::string> split;
		boost::algorithm::split(split, line, boost::algorithm::is_any_of(":"));

		if (split.size() != 2) {
			std::cout << "PinType value is wrong." << std::endl;
			exit(-1);
		}

		type_map[split[0]] = split[1];
	}
	g_data::set_type_map(type_map);

	for (auto it : serial_map) {
		g_data::log(INFO_LOG_LEVEL, "[main] PIN : %s, SERIAL : %s", it.first.c_str(), it.second.c_str());
	}

	for (auto it : type_map) {
		g_data::log(INFO_LOG_LEVEL, "[main] PIN : %s, TYPE : %s", it.first.c_str(), it.second.c_str());
	}

	std::string log_level = cfg.str("CONFIG", "LOG_LEVEL", "DEBUG");
	if (log_level == "DEBUG") {
		g_data::set_log_level(DEBUG_LOG_LEVEL);
	} else if (log_level == "INFO") {
		g_data::set_log_level(INFO_LOG_LEVEL);
	} else if (log_level == "WARN") {
		g_data::set_log_level(WARN_LOG_LEVEL);
	} else if (log_level == "ERROR") {
		g_data::set_log_level(ERROR_LOG_LEVEL);
	}
}

int main(int argc, char **argv) {
	int n = 0;
	std::string config_path = "../config/spi2mqtt.ini";
	while ((n = getopt(argc, argv, "hi:")) != EOF) {
		switch (n) {
			case 'h':
				break;
			case 'i':
				config_path = optarg;
				break;
			default:
				break;
		}
	}

	g_data::init();
	g_data::set_log_level(DEBUG_LOG_LEVEL);

	init_config(config_path.c_str());

	mqtt_agent mq_agent_;
	spi_slave spi_slave_;

	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}
