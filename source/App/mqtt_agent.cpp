#include "mqtt_agent.h"

mqtt_agent::mqtt_agent() {
	pin_serial_map_ = g_data::serial_map();
	pin_type_map_ = g_data::type_map();
	make_unit_map();
	make_data_type_map();
    mq_info_ = g_data::mq_info();

    mq_pub_id_ = "spi2mqtt_pub_cli";
    mq_sub_id_ = "spi2mqtt_sub_cli";

    pub_client_ = std::make_shared<mqtt_publisher>(mq_info_.mqtt_address.c_str(), mq_info_.mqtt_user.c_str(), 
        mq_info_.mqtt_password.c_str(), mq_pub_id_.c_str());
    pub_client_->connect();

    sub_client_ = std::make_shared<mqtt_subscriber>(mq_info_.mqtt_address.c_str(), mq_info_.mqtt_user.c_str(), 
        mq_info_.mqtt_password.c_str(), mq_sub_id_.c_str());

    // todo set topic
	std::vector<std::string> topic;
	topic.push_back(mq_info_.subs_topic);
	std::vector<int> qos { 0 };

	sub_client_->set_topic(topic, qos);
    sub_client_->connect();

    subs_run_flag_ = true;
    mq_subs_th_ = std::thread(&mqtt_agent::mqtt_subs_th, this);

    pubs_run_flag_ = true;
    mq_pubs_th_ = std::thread(&mqtt_agent::mqtt_pubs_th, this);
}

mqtt_agent::~mqtt_agent() {
    subs_run_flag_ = false;
    if (mq_subs_th_.joinable()) {
        mq_subs_th_.join();
    }

    pubs_run_flag_ = false;
    if (mq_pubs_th_.joinable()) {
        mq_pubs_th_.join();
    }
}

void mqtt_agent::make_unit_map() {
	unit_map_["soilh"] = "%";
	unit_map_["temp"] = "°C";
	unit_map_["humi"] = "%";
	unit_map_["pm10"] = "μg/m³";
	unit_map_["pm25"] = "μg/m³";
}

void mqtt_agent::make_data_type_map() {
	data_type_map_["soilh"] = "float";
	data_type_map_["valve"] = "string";
	data_type_map_["temp"] = "int";
	data_type_map_["humi"] = "int";
	data_type_map_["pm10"] = "long";
	data_type_map_["pm25"] = "long";
}

void mqtt_agent::mqtt_subs_th() {
    zmq::socket_t sock(sub_client_->context(), zmq::socket_type::pull);
	sock.connect(ZMQ_MQTT_SUBSCRIBE_ADDRESS);

    Json::Reader reader;
    while (subs_run_flag_) {
        zmq::message_t msg;
		auto res = sock.recv(msg, zmq::recv_flags::dontwait);
		if (res && res.value() > 0) {
            std::string data(msg.data<char>(), msg.size());
			Json::Value root;
			reader.parse(data, root);
			g_data::log(INFO_LOG_LEVEL, "[mqtt_agent] Subscribe topic[%s] data[%s]", root["topic"].asCString(), root["payload"].asCString());

			Json::Value payload;
			reader.parse(root["payload"].asString(), payload);

			std::string serial = payload["s"].asString();
			std::string method = payload["m"].asString();

			std::string pin;
			std::map<std::string, std::string> pin_map = g_data::serial_map();
			for (auto it : pin_map) {
				if (it.second == serial) {
					pin = it.first;
					break;
				}
			}
			if (pin.empty()) {
				g_data::log(ERROR_LOG_LEVEL, "[mqtt_agent] Unknown serial[%s]", serial.c_str());
				continue;
			}

			spi_data spi;
			snprintf(spi.pin, sizeof(spi.pin), "%s", pin.c_str());
			if (method == "on") {
				int value = 1;
				std::memcpy(spi.value, &value, sizeof(spi.value));
			} else {
				int value = 0;
				std::memcpy(spi.value, &value, sizeof(spi.value));
			}

			zmq::message_t spi_message(&spi, sizeof(spi_data));
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

void mqtt_agent::mqtt_pubs_th() {
    sock_ = std::make_shared<zmq::socket_t>(g_data::context(), zmq::socket_type::pull);
    sock_->connect(g_data::zmq_mq_address());

    while (pubs_run_flag_) {
        zmq::message_t msg;
        auto res = sock_->recv(msg, zmq::recv_flags::dontwait);
        if (res && res.value() > 0) {
			spi_data *data = msg.data<spi_data>();
			auto it_serial = pin_serial_map_.find(data->pin);
			if (it_serial == pin_serial_map_.end()) {
				g_data::log(WARN_LOG_LEVEL, "[mqtt_agent] Pin[%s] is wrong pin number[unknown serial]", data->pin);
				continue;
			}

			auto it_pin_type = pin_type_map_.find(data->pin);
			if (it_pin_type == pin_type_map_.end()) {
				g_data::log(WARN_LOG_LEVEL, "[mqtt_agent] Pin[%s] is wrong pin number[unknown type]", data->pin);
				continue;
			}

			auto it_unit = unit_map_.find(it_pin_type->second);
			auto it_data_type = data_type_map_.find(it_pin_type->second);
			if (it_data_type == data_type_map_.end()) {
				g_data::log(WARN_LOG_LEVEL, "[mqtt_agent] PinType is wrong.[%s]", it_pin_type->second.c_str());
				continue;
			}

			Json::Value root;
			root["s"] = it_serial->second;
			if (it_unit == unit_map_.end()) {
				Json::Value null;
				root["u"] = null;
			} else {
				root["u"] = it_unit->second;
			}
			root["d"] = it_data_type->second;
			root["k"] = it_pin_type->second;

			if (it_data_type->second == "int" or it_data_type->second == "long") {
				int value = 0;
				memcpy(&value, data->value, sizeof(int));
				root["v"] = value;
			} else if (it_data_type->second == "float") {
				float value = 0.0;
				memcpy(&value, data->value, sizeof(float));
				root["v"] = value;
			} else if (it_data_type->second == "string") {
				int temp = 0;
				memcpy(&temp, data->value, sizeof(int));
				root["v"] = temp == 1 ? "on" : "off";
			}
			root["c"] = 1;

			Json::FastWriter writer;
			std::string json_string = writer.write(root);

			pub_client_->publish(mq_info_.pub_topic, json_string, 0);
			g_data::log(DEBUG_LOG_LEVEL, "[mqtt_agent] Publish data topic[%s] data[%s]", mq_info_.pub_topic.c_str(), json_string.c_str());
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
    }
}