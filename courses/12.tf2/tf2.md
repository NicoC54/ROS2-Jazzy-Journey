# Comprendre la librairie tf2 dans ROS 2

## 1. Concepts Fondamentaux
*   **Frame (Repère) :** Un point de référence dans l'espace 3D défini par des coordonnées (X,Y,Z) et une orientation (quaternion). Exemple : `base_link` (centre du robot), `laser_link` (position du Lidar).
*   **Transform (TF) :** Le vecteur mathématique permettant de passer d'un repère A (parent) à un repère B (enfant). Composé d'une translation (mètres) et d'une rotation (quaternions).
*   **TF Tree :** La structure de données (graphe acyclique dirigé) qui relie tous les repères. 
    *   *Règle stricte :* Un repère enfant a **1 et 1 seul parent**. Un parent peut avoir plusieurs enfants.

## 2. Le Temps et l'Historique
tf2 n'est pas juste spatial, il est **temporel**. 
Chaque transformation publiée contient un *timestamp* (horodatage). Le système stocke l'historique des TFs (généralement pendant 10 secondes) dans un **TF Buffer**. Cela permet d'interroger la position d'un capteur *dans le passé*, au moment exact où il a capturé la donnée, pour synchroniser l'ensemble du système.

## 3. Les Broadcasters (Émetteurs)
*   **Static Broadcaster (`tf2_ros::StaticTransformBroadcaster`) :** Pour les pièces fixes du robot (ex: capteur monté sur le châssis). Publié très peu de fois (souvent en *Transient Local*) pour économiser le CPU et le réseau.
*   **Dynamic Broadcaster (`tf2_ros::TransformBroadcaster`) :** Pour les pièces mobiles (bras robotique) ou le déplacement du robot dans le monde. Publié en continu (ex: 50 Hz).

## 4. L'Architecture Standard (REP 105)
L'arbre standard pour la navigation mobile est : `map` $\rightarrow$ `odom` $\rightarrow$ `base_link`.
1.  **`base_link` :** Le corps rigide du robot.
2.  **`odom` (Odometry) :** Le repère parent de `base_link`. Calculé via les capteurs internes (encodeurs de roues, IMU). C'est un repère **continu et lisse**, indispensable pour le contrôle moteur local. Il ne fait jamais de "sauts", mais il s'accumule de la dérive sur le long terme.
3.  **`map` :** Le repère global, parent de `odom`. Calculé par le SLAM (laser/caméra). C'est la vérité absolue. Il a le droit de faire des sauts discontinus pour corriger les erreurs accumulées dans `odom`.
*La TF `map` $\rightarrow$ `odom` est publiée par le SLAM pour compenser la dérive du robot.*

---

## 5. Exemples de Code C++ (ROS 2 Jazzy)

### A. Diffuser une TF Statique (Static Broadcaster)
Utile pour définir la position d'un laser par rapport au centre du robot.

```cpp
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

#include "rclcpp/rclcpp.hpp"
#include "tf2_ros/transform_listener.h"
#include "tf2_ros/buffer.h"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2/exceptions.h"

class TfListenerNode : public rclcpp::Node {
public:
    TfListenerNode() : Node("tf_listener") {
        // 1. Créer le Buffer (la mémoire spatio-temporelle)
        tf_buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());
        
        // 2. Créer le Listener (qui remplit le buffer automatiquement en écoutant le réseau)
        tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

        // Créer un timer pour vérifier la position toutes les secondes
        timer_ = this->create_wall_timer(
            std::chrono::seconds(1),
            std::bind(&TfListenerNode::on_timer, this));
    }

private:
    void on_timer() {
        try {
            // Chercher la transformation de "laser_link" vers "map"
            // tf2::TimePointZero signifie "donne-moi la TF la plus récente disponible"
            geometry_msgs::msg::TransformStamped transformStamped = 
                tf_buffer_->lookupTransform(
                    "map",           // Target frame (Je veux les coordonnées dans ce repère)
                    "laser_link",    // Source frame (L'objet que je cherche)
                    tf2::TimePointZero);

            RCLCPP_INFO(this->get_logger(), "Le laser est à X: %f par rapport à la map", 
                        transformStamped.transform.translation.x);
        }
        catch (const tf2::TransformException & ex) {
            // Toujours attraper l'exception car la TF peut ne pas encore être disponible
            RCLCPP_WARN(this->get_logger(), "Impossible de trouver la TF: %s", ex.what());
        }
    }

    std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
    std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
    rclcpp::TimerBase::SharedPtr timer_;
};