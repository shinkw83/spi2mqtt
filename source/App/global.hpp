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

typedef struct mqtt_conn_info {
	std::string mqtt_address_;
    std::string mqtt_user_;
	std::string mqtt_password_;
} mqtt_conn_info;

class g_data : public singleton_T<g_data> {
public:
	g_data() : ctx_() {
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		std::chrono::system_clock::duration tp = now.time_since_epoch();
		tp -= std::chrono::duration_cast<std::chrono::seconds>(tp);

		time_t tt = std::chrono::system_clock::to_time_t(now);
		struct tm *t = localtime(&tt);

		char path[1024] = {0, };
		snprintf(path, sizeof(path), "log/spi2mqtt_%04u-%02u-%02u_%02u:%02u:%02u",
				t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min,
				t->tm_sec);

		logmgr_ = new LogMgrC(path, "spi2mqtt");

		zmq_mq_address_ = "inproc://pub_mqtt_proc";
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

	static void set_mqtt_info(const mqtt_conn_info &info) {
		g_data *data = g_data::GetInstance();
		data->mq_con_info_ = info;
	}

	static mqtt_conn_info &mq_info() {
		g_data *data = g_data::GetInstance();
		return data->mq_con_info_;
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

	LogMgrC *logmgr_ = nullptr;

	mqtt_conn_info mq_con_info_;
};
