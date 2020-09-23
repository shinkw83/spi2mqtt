#include "spi_slave.h"

spi_slave::spi_slave() {
	init_spi();

	mq_sock_ = std::make_shared<zmq::socket_t>(g_data::context(), zmq::socket_type::push);
	mq_sock_->bind(g_data::zmq_mq_address());

	spi_sock_ = std::make_shared<zmq::socket_t>(g_data::context(), zmq::socket_type::pull);
	spi_sock_->bind(g_data::zmq_spi_address());

	send_run_flag_ = true;
	recv_run_flag_ = true;

	send_th_ = std::thread(&spi_slave::mq_send_proc, this);
	recv_th_ = std::thread(&spi_slave::mq_recv_proc, this);
}

spi_slave::~spi_slave() {
	send_run_flag_ = false;
	recv_run_flag_ = false;

	if (send_th_.joinable()) {
		send_th_.join();
	}

	if (recv_th_.joinable()) {
		recv_th_.join();
	}
}

void spi_slave::init_spi() {
	spi_info spi = g_data::spi();
	fd_ = open(spi.device.c_str(), O_RDWR);
	if (fd_ < 0) {
		g_data::log(ERROR_LOG_LEVEL, "[spi_slave] Can't open device[%s]", spi.device.c_str());
		exit(-1);
	}

	int mode = 0;
	int ret = 0;

	ret = ioctl(fd_, SPI_IOC_WR_MODE, &mode);
	if (ret == -1) {
		g_data::log(ERROR_LOG_LEVEL, "[spi_slave] Can't set spi mode.");
		exit(-1);
	}
	ret = ioctl(fd_, SPI_IOC_RD_MODE, &mode);
	if (ret == -1) {
		g_data::log(ERROR_LOG_LEVEL, "[spi_slave] Can't get spi mode.");
		exit(-1);
	}

	ret = ioctl(fd_, SPI_IOC_WR_BITS_PER_WORD, &bit_per_word_);
	if (ret == -1) {
		g_data::log(ERROR_LOG_LEVEL, "[spi_slave] Can't set bits per word.");
		exit(-1);
	}	
	ret = ioctl(fd_, SPI_IOC_RD_BITS_PER_WORD, &bit_per_word_);
	if (ret == -1) {
		g_data::log(ERROR_LOG_LEVEL, "[spi_slave] Can't get bits per word.");
		exit(-1);
	}

	ret = ioctl(fd_, SPI_IOC_WR_MAX_SPEED_HZ, &speed_);
	if (ret == -1) {
		g_data::log(ERROR_LOG_LEVEL, "[spi_slave] Can't set max speed hz.");
		exit(-1);
	}
	ret = ioctl(fd_, SPI_IOC_RD_MAX_SPEED_HZ, &speed_);
	if (ret == -1) {
		g_data::log(ERROR_LOG_LEVEL, "[spi_slave] Can't get max speed hz.");
		exit(-1);
	}
}

void spi_slave::mq_send_proc() {
	while (send_run_flag_) {
		spi_data data;
		uint8_t *rx = (uint8_t *)&data;
		uint8_t *tx = nullptr;

		struct spi_ioc_transfer tr;
		memset(&tr, 0, sizeof(tr));
		tr.tx_buf = (unsigned long)tx;
		tr.rx_buf = (unsigned long)rx;
		tr.len = sizeof(spi_data);
		tr.delay_usecs = delay_;
		tr.speed_hz = speed_;
		tr.bits_per_word = bit_per_word_;

		int ret = ioctl(fd_, SPI_IOC_MESSAGE(1), &tr);
		if (ret < 1) {
			g_data::log(ERROR_LOG_LEVEL, "[spi_slave] Can't read spi message.");
			continue;
		}

		// check spi data
		if (data.type == 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			continue;
		}

		zmq::message_t message(&data, sizeof(spi_data));
		mq_sock_->send(message, zmq::send_flags::dontwait);
	}
}

void spi_slave::mq_recv_proc() {
	while (recv_run_flag_) {
		zmq::message_t message;
		auto res = spi_sock_->recv(message, zmq::recv_flags::dontwait);
		if (res && res.value() > 0) {
			spi_data *data = message.data<spi_data>();

			uint8_t *tx = (uint8_t *)data;
			uint8_t *rx = nullptr;

			struct spi_ioc_transfer tr;
			memset(&tr, 0, sizeof(tr));
			tr.tx_buf = (unsigned long)tx;
			tr.rx_buf = (unsigned long)rx;
			tr.len = sizeof(spi_data);
			tr.delay_usecs = delay_;
			tr.speed_hz = speed_;
			tr.bits_per_word = bit_per_word_;

			int ret = ioctl(fd_, SPI_IOC_MESSAGE(1), &tr);
			if (ret < 1) {
				g_data::log(ERROR_LOG_LEVEL, "[spi_slave] Can't send spi message.");
			}
		} else {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}
}