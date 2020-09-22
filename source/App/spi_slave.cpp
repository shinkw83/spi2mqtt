#include "spi_slave.h"

spi_slave::spi_slave() {
	spi_info spi = g_data::spi();
	channel_ = spi.channel;

	sock_ = std::make_shared<zmq::socket_t>(g_data::context(), zmq::socket_type::push);
	sock_->bind(g_data::zmq_mq_address());
}

spi_slave::~spi_slave() {
	Stop();
}

int spi_slave::Proc() {
	spi_data data;
	unsigned char *p = (unsigned char *)&data;
	wiringPiSPIDataRW(channel_, p, sizeof(spi_data));

	// todo check spi data

	zmq::message_t message(&data, sizeof(spi_data));
	sock_->send(message, zmq::send_flags::dontwait);
	return 0;
}