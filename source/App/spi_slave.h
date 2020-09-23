#pragma once

#include "global.hpp"
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

class spi_slave {
public:
	spi_slave();
	~spi_slave();

private:
	void init_spi();
	void mq_send_proc();
	void mq_recv_proc();

private:
	const uint8_t bit_per_word_ = 8;
	const uint16_t delay_ = 0;

private:
	int fd_;
	uint32_t speed_;

	std::shared_ptr<zmq::socket_t> mq_sock_;
	std::shared_ptr<zmq::socket_t> spi_sock_;

	std::thread send_th_;
	std::thread recv_th_;

	std::atomic<bool> send_run_flag_;
	std::atomic<bool> recv_run_flag_;
};