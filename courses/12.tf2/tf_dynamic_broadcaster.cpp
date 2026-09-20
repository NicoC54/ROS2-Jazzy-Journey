    #include "rclcpp/rclcpp.hpp"
    #include "tf2_ros/transform_broadcaster.h"
    #include "geometry_msgs/msg/transform_stamped.hpp"
    #include <cmath>



    class DynamicTransformBroadcaster : public rclcpp::Node{

        public:
            DynamicTransformBroadcaster() : Node("dynamic_transform_broadcaster"){


                dynamic_transform_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);
                timer_ = this->create_wall_timer(std::chrono::milliseconds(20), std::bind(&DynamicTransformBroadcaster::TimerCallback, this));

                }

                void TimerCallback(){
                    geometry_msgs::msg::TransformStamped t;
                    t.header.stamp = this->get_clock()->now();
                    t.header.frame_id = "odom";
                    t.child_frame_id = "base_link";

                    double time_sec = this->get_clock()->now().seconds();

                    t.transform.translation.x = 0.5*time_sec;
                    t.transform.translation.y = 0;
                    t.transform.translation.z = 0;

                    t.transform.rotation.x = 0;
                    t.transform.rotation.y = 0;
                    t.transform.rotation.z = 0;
                    t.transform.rotation.w = 1;

                    dynamic_transform_broadcaster_ -> sendTransform(t);

                }

        private:
                std::shared_ptr<rclcpp::TimerBase> timer_;
                std::shared_ptr<tf2_ros::TransformBroadcaster> dynamic_transform_broadcaster_;

            

    };