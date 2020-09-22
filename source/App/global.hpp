#pragma once

#include "singleton.h"
#include "xthread.h"
#include "json.h"
#include "xpacket.h"
#include "config.h"
#include <zmq.hpp>
#include <vector>
#include <map>
#include <mutex>
#include <boost/asio.hpp>
#include "logger.hpp"
#include <stdarg.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

typedef struct mqtt_conn_info {
	std::string mqtt_address;
    std::string mqtt_user;
	std::string mqtt_password;
	std::string pub_topic;
	std::string subs_topic;
} mqtt_conn_info;

typedef struct spi_info {
	int channel;
	int speed;

	spi_info() {
		channel = 0;
		speed = 1000000;
	}
} spi_info;

typedef struct spi_data {
	char type;
	char pin[4];
	char value[4];

	spi_data() {
		type = 0;
		memset(pin, 0x00, sizeof(pin));
		memset(value, 0x00, sizeof(value));
	}
} spi_data;

class g_data : public singleton_T<g_data> {
public:
	g_data() : ctx_() {
		logmgr_ = new LogMgrC("log/spi2mqtt.log", "spi2mqtt");

		zmq_mq_address_ = "inproc://pub_mqtt_proc";
		zmq_spi_address_ = "inproc://spi_proc";
	}

	virtual ~g_data() {
		delete logmgr_;
	}

	static void init() {
		__attribute__((unused)) g_data *data = g_data::GetInstance();
	}

	static zmq::context_t &context() {
		g_data *data = g_data::GetInstance();
		return data->ctx_;
	}

	static const char *zmq_mq_address() {
		g_data *data = g_data::GetInstance();
		return data->zmq_mq_address_.c_str();
	}

	static const char *zmq_spi_address() {
		g_data *data = g_data::GetInstance();
		return data->zmq_spi_address_.c_str();
	}

	static void set_mqtt_info(const mqtt_conn_info &info) {
		g_data *data = g_data::GetInstance();
		data->mq_con_info_ = info;
	}

	static mqtt_conn_info &mq_info() {
		g_data *data = g_data::GetInstance();
		return data->mq_con_info_;
	}

	static void set_spi_info(const spi_info &info) {
		g_data *data = g_data::GetInstance();
		data->spi_info_ = info;
	}

	static const spi_info &spi() {
		g_data *data = g_data::GetInstance();
		return data->spi_info_;
	}

	static void set_serial_map(const std::map<std::string, std::string> &serial_map) {
		g_data *data = g_data::GetInstance();
		data->serial_map_ = serial_map;
	}

	static const std::map<std::string, std::string> &serial_map() {
		g_data *data = g_data::GetInstance();
		return data->serial_map_;
	}

	static void set_type_map(const std::map<std::string, std::string> &type_map) {
		g_data *data = g_data::GetInstance();
		data->type_map_ = type_map;
	}

	static const std::map<std::string, std::string> &type_map() {
		g_data *data = g_data::GetInstance();
		return data->type_map_;
	}

	static void set_log_level(int level) {
		g_data *data = g_data::GetInstance();
		data->logmgr_->setLogLevel(level);
	}

	static void log(int level, const char *format, ...) {
		g_data *data = g_data::GetInstance();
		if (format == nullptr) return;
		if (data->logmgr_ == nullptr) return;

		char logstr[256] = {0, };
		va_list ap;
		va_start(ap, format);
		vsnprintf(logstr, sizeof(logstr), format, ap);
		va_end(ap);

		switch (level) {
		case TRACE_LOG_LEVEL:
			LOG4CPLUS_TRACE(data->logmgr_->get(), logstr);
				break;
		case DEBUG_LOG_LEVEL:
			LOG4CPLUS_DEBUG(data->logmgr_->get(), logstr);
			break;
		case INFO_LOG_LEVEL:
			LOG4CPLUS_INFO(data->logmgr_->get(), logstr);
			break;
		case WARN_LOG_LEVEL:
			LOG4CPLUS_WARN(data->logmgr_->get(), logstr);
			break;
		default:
			LOG4CPLUS_ERROR(data->logmgr_->get(), logstr);
			break;
		}
	}

private:
	zmq::context_t ctx_;

	std::string zmq_mq_address_;
	std::string zmq_spi_address_;

	LogMgrC *logmgr_ = nullptr;

	mqtt_conn_info mq_con_info_;

	spi_info spi_info_;

	std::map<std::string, std::string> serial_map_;
	std::map<std::string, std::string> type_map_;
};
