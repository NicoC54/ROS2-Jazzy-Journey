#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/quaternion.hpp"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2_geometry_msgs/tf2_geimetry_msgs.hpp"

void example_rotations(){
    double roll = 0;
    double pitch =0;
    double yaw = 1.57;

    tf2::Quaternion tf2_quat;

    tf2_quat.setRPY(roll,pitch,yaw);

    geometry_msgs::msg::Quaternion msg_quat;

    msg_quat = tf2::toMsg(tf2_quat);
}

void reverse_example(geometry_msgs::msg::Quaternion incoming_msg){

    tf2::Quaternion tf2_quat;
    tf2::fromMsg(incoming_msg, tf2_quat);

    tf2::matrix3x3 matrix(tf2_quat);
    double roll, pitch, yaw;
    matrix.getRPY(roll,pitch,yaw);

}





//transformer un angle deuler en message quaternion
void EulerToQuat(){
    double roll = 0.0;
    double pitch = 0.0;
    double yaw = 1.5708; // 90 degrés en radians

    //creation de l'objet quaternion
    tf2::Quaternion quat_obj;
    //remplissage de lobjet quaternion
    quat_obj.setRPY(roll,pitch,yaw);
    
    //déclaration d'un message quaternion
    geometry_msgs::msg::Quaternion msg_quat;
    msg_quat = toMsg(quat_obj);

}

//transformer un message quaternion en euler
void QuatToEuler(const geometry_msgs::msg::Quaternion msg_quat){

    tf2::Quaternion quat;
    tf2::fromMsg(msg_quat, quat);
    tf2::matrix3x3 matrix(quat);
    double roll,pitch,yaw;
    matrix.getRPY(roll,pitch,yaw)

}


void QuatToEuler(geometry_msgs::msg::Quaternion quat_msg){

    tf2::Quaternion quat;
    tf2::fromMsg(quat_msg,quat)
    tf2::matrix3x3 matrix(quat);
    double roll{}, pitch{}, yaw{};
    matrix.getRPY(roll,pitch,yaw);

}