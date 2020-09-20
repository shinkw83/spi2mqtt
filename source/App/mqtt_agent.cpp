#include "mqtt_agent.h"

mqtt_agent::mqtt_agent() {
    mq_info_ = g_data::mq_info();

    mq_pub_id_ = "spi2mqtt_pub_cli";
    mq_sub_id_ = "spi2mqtt_sub_cli";

    pub_client_ = std::make_shared<mqtt_publisher>(mq_info_.mqtt_address_.c_str(), mq_info_.mqtt_user_.c_str(), 
        mq_info_.mqtt_password_.c_str(), mq_pub_id_.c_str());
    pub_client_->connect();

    sub_client_ = std::make_shared<mqtt_subscriber>(mq_info_.mqtt_address_.c_str(), mq_info_.mqtt_user_.c_str(), 
        mq_info_.mqtt_password_.c_str(), mq_sub_id_.c_str());
    // todo set topic
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

        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
    }
}