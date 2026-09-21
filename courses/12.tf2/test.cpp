#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2_ros/transform_listener.hpp"
#include "tf2_ros/transform_broadcaster.hpp" // MANQUANT : Nécessaire pour le broadcaster
#include "tf2_ros/buffer.hpp"
#include "tf2/exceptions.h"                  // MANQUANT : Nécessaire pour tf2::TransformException
#include <chrono>

class Tf2Listener : public rclcpp::Node {



    //partie listener

    public:
        
        Tf2Listener() : Node("Tf2Listener"){

            timer_ = this->create_wall_timer(std::chrono::seconds(1),std::bind(&Tf2BroadcasterAndListener::ChronoCallback, this));

            buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());

            listener_ = std::make_shared<tf2_ros::TransformListener>(*buffer_);
              }


    private:


            void ChronoCallback(){

                try{

                    geometry_msgs::msg::TransformStamped tf = buffer_->lookupTransform("map", "base_link", tf2::TimePointZero);
                }
                catch(const tf2::TransformException& ex){

                    RCLCPP_WARN(this->get_logger,"La transform n'est pas disponible: %s", ex.what());

            }
        }
            
            std::shared_ptr<rclcpp::TimerBase> timer_;
            std::unique_ptr<tf2_ros::Buffer> buffer_;
            std::shared_ptr<tf2_ros::TransformListener> listener_;

    };

class Tf2DynamicBroadcaster : public rclcpp::Node {

public:
    Tf2DynamicBroadcaster() : Node("Dynamic_broadcaster"), x_pos_(0.0){

        dynamic_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);
        timer_ = create_wall_timer(std::chrono::seconds(1),&Tf2DynamicBroadcaster::TimerCallback,this);
        }

        void TimerCallback(){

        geometry_msgs::msg::TransformStamped tf;
        tf.header.stamp = this->get_clock()->now();
        tf.header.frame_id = "map";
        tf.child_frame_id = "base_link";

        
        double vitesse = 5;
        x_pos_ += vitesse;

        //Simu robot qui avance en ligne droite

        tf.tranform.translation.x = x_pos;
        tf.transform.translation.y = 0;
        tf.transform.translation.z = 0;
        tf.transform.rotation.x = 0;
        tf.transform.rotation.y = 0;
        tf.transform.rotation.z = 0;
        tf.transform.rotation.w = 1;

        dynamic_broadcaster_ ->sendTransform(tf);
        
        }


private:
    std::shared_ptr<tf2_ros::TransformBroadcaster> dynamic_broadcaster_;
    std::shared_ptr<rclcpp::TimerBase> timer_;
    double x_pos_;

};


int main(int argc, char* argv[]){
    rclcpp::init(argc, argv);
    rclcpp::executors::MultiThreadedExecutor executor;
    auto broadcaster = std::make_shared<Tf2DynamicBroadcaster>();
    auto listener = std::make_shared<Tf2Listener>();

    executor.add_node(broadcaster);
    executor.add_node(listener);
    
    executor.spin();
    rclcpp::shutdown();
    return 0;
}



rclcpp::executors::MultiThreadedExecutor executor;

auto broadcaster = std::make_shared<Tf2DynamicBroadcaster>();
auto listener = std::make_shared<Tf2Listener>();
executor.add_node(broadcaster);
executor.add_node(listener);
executor.spin();