#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include <chrono>
using namespace std::chrono_literals;

class MonPublisher : public rclcpp::Node{

    public:

    MonPublisher() : Node("Mon_noeud"),compte_(0)

    {
    publisher_ = this-> create_publisher<std_msgs::msg::String>( "mon_topic", 10);
    timer_ = this-> create_wall_timer(500ms, std::bind(&MonPublisher::Callback, this));
    }

    private:

    void Callback(){
        auto msg = std_msgs::msg::String();
        msg.data = "publication message";
        publisher_->publish(msg);
        }

        rclcpp::TimerBase::SharedPtr timer_;
        rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_; 
        size_t compte_;

};


int main(int argc, char* argv[]){


    rclcpp::init(argc,argv);
    rclcpp::spin(std::make_shared<MonPublisher>());
    rclcpp::shutdown();
    return 0;
}



