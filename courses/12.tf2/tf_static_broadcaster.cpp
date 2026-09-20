#include "rclcpp/rclcpp.hpp"
#include "tf2_ros/static_transform_broadcaster.h"
#include "geometry_msgs/msg/transform_stamped.hpp"

class StaticTfPublisher : public rclcpp::Node {
public:
    StaticTfPublisher() : Node("static_tf_publisher") {
        // Initialiser le broadcaster statique
        tf_static_broadcaster_ = std::make_shared<tf2_ros::StaticTransformBroadcaster>(this);

        // Créer le message
        geometry_msgs::msg::TransformStamped t;
        t.header.stamp = this->get_clock()->now();
        t.header.frame_id = "base_link"; // Parent
        t.child_frame_id = "laser_link"; // Enfant

        // Translation (ex: laser situé 20cm devant et 10cm plus haut)
        t.transform.translation.x = 0.2;
        t.transform.translation.y = 0.0;
        t.transform.translation.z = 0.1;

        // Rotation (utilisation des quaternions, ici pas de rotation)
        t.transform.rotation.x = 0.0;
        t.transform.rotation.y = 0.0;
        t.transform.rotation.z = 0.0;
        t.transform.rotation.w = 1.0; // w=1 et xyz=0 signifie "aucune rotation"

        // Envoyer la TF
        tf_static_broadcaster_->sendTransform(t);
    }
private:
    std::shared_ptr<tf2_ros::StaticTransformBroadcaster> tf_static_broadcaster_;
};
