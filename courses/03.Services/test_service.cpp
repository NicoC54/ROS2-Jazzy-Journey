#include "std_msgs/msg/String.hpp"
#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/srv/add_two_ints.hpp"
#include <chrono>
#include <memory>


class Service : public rclcpp::Node{

    public:

        Service() : Node("Service_node")
        {
            service_ = this->create_service<example_interfaces::srv::AddTwoInts>("nom_du_canal", std::bind(&Service::ProceedRequestAndSendResponse, this, std::placeholders::_1, std::placeholders::_2));
        }

    private:


        void ProceedRequestAndSendResponse(const std::shared_ptr<example_interfaces::srv::AddTwoInts::Request> request, std::shared_ptr<example_interfaces::srv::AddTwoInts::Response> response){
            RCLCPP_INFO(this->get_logger(),"réception de la requete et calcul de la réponse");
            response->sum = request->a + request->b;

        }

        rclcpp::Service<example_interfaces::srv::AddTwoInts>::SharedPtr service_;


};


int main(int argc, char* argv[]){
    rclcpp::init(argc,argv);
    rclcpp::spin(std::make_shared<Service>());
    rclcpp::shutdown();

    return 0;
}

/*

📝 Erreurs sur le Nœud Service

    Portée des types (Les chevrons) : Comme dans le client, dans les paramètres de ton callback, tu avais écrit std::shared_ptr<...>::Request au lieu de std::shared_ptr<...::Request>.

    API create_service : Tu avais ajouté un 10 (taille de la file d'attente). Contrairement à un Publisher (topic) qui en a besoin, la création d'un service basique ne prend que le nom du canal et la fonction de callback.

    Orthographe des macros C++ : Tu as écrit std::spaceholders au lieu de std::placeholders (les espaces réservés pour la fonction bind).

    Étourderies classiques : Tu as déclaré deux fois la variable service_ dans ta zone privée, et oublié un point-virgule après rclcpp::init(argc,argv).*/