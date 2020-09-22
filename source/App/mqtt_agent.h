#pragma once

#include "global.hpp"
#include "publisher.h"
#include "subscriber.h"

class mqtt_agent {
public:
    mqtt_agent();
    ~mqtt_agent();

private:
    void mqtt_subs_th();
    void mqtt_pubs_th();

	void make_unit_map();
	void make_data_type_map();

private:
    std::shared_ptr<zmq::socket_t> sock_;
    std::shared_ptr<zmq::socket_t> subs_sock_;

    std::shared_ptr<mqtt_publisher> pub_client_;
    std::shared_ptr<mqtt_subscriber> sub_client_;

    mqtt_conn_info mq_info_;
	std::string mq_pub_id_;
    std::string mq_sub_id_;

    std::thread mq_subs_th_;
    std::atomic<bool> subs_run_flag_;

    std::thread mq_pubs_th_;
    std::atomic<bool> pubs_run_flag_;

	std::map<std::string, std::string> pin_serial_map_;
	std::map<std::string, std::string> pin_type_map_;
	std::map<std::string, std::string> unit_map_;
	std::map<std::string, std::string> data_type_map_;

};