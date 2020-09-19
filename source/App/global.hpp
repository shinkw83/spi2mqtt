#pragma once

#include "singleton.h"
#include "xthread.h"
#include "json.h"
#include "xpacket.h"
#include <zmq.hpp>
#include <vector>
#include <map>
#include <mutex>
#include <boost/asio.hpp>
#include "logger.hpp"
#include <stdarg.h>

class g_data : public singleton_T<g_data> {
public:
	g_data() : ctx_() {
		logmgr_ = new LogMgrC("spi2mqtt_log.log", "spi2mqtt");
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

	LogMgrC *logmgr_ = nullptr;
};
