#include "rclcpp/rclcpp.hpp"
#include "tf2_ros/transform_listener.h"
#include "tf2_ros/buffer.h"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2/exceptions.h"


class TFListenerNode : public rclcpp::Node{

    public: 

        TFListenerNode() : Node("TfListener_node"){


            //creation d'un tf_buffer, qui va stocker les transformées avec leur stamp, on lui donne l'objet horloge car on ne veut pas un instant présent uniquement
            tf_buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());


            //écoute automatique de toutes les transform et insersion dans *tf_buffer
            tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

            //Creation du timer, lecture du buffer tous les combien de temps
            timer_ = this->create_wall_timer(std::chrono::seconds(1),std::bind(&TFListenerNode::ChronoCallback,this));



        }
    private:

    void ChronoCallback(){
        try{
            //déclaration et instanciation d'un tf égal à la dernière tf ajoutée (la plus récente) présente dans le buffer
            geometry_msgs::msg::TransformStamped LastTransform = tf_buffer_ -> lookupTransform("map", "laser_link", tf2::TimePointZero);


            RCLCPP_INFO(this->get_logger(), "Le laser est à X: %f par rapport à la map", LastTransform.transform.translation.x);
        }

        catch (const tf2::TransformException & ex) {
            // Toujours attraper l'exception car la TF peut ne pas encore être disponible
            RCLCPP_WARN(this->get_logger(), "Impossible de trouver la TF: %s", ex.what());
    }
    }
    std::shared_ptr<rclcpp::TimerBase> timer_;
    std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
    std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
};


