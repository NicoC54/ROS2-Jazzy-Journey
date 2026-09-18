


class MonSubscriber : public rclcpp::Node {


public :

    MonSubscriber() : Node("noeud de recption") 
    {
    subscriber_ = this->create_subscription<std_msgs::msg::String>("nom_du_topic", 10,std::bind(&MonSubscriber::callback,this,std::placeholders::_1));
    }


private:

    void callback(const std_msgs::msg::String& msg) const 
    
    {
    RCLCPP_INFO(this->get_logger(), "message recu: %s" , msg.data.c_str());

    }

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscriber_;
};

int main (int argc, char* argv[]){
    rclcpp::init(argc,argv);
    rclcpp::spin(std::make_shared<MonSubscriber>());
    rclcpp::shutdown();
    return 0;

}