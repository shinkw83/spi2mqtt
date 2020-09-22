#pragma once

#include "global.hpp"

class spi_slave : public xthread {
public:
	spi_slave();
	virtual ~spi_slave();

private:
	virtual int Proc();

private:
	int channel_;

	std::shared_ptr<zmq::socket_t> sock_;
};