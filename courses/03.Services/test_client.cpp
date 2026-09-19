#include "std_msgs/msg/String.hpp"
#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/srv/add_two_ints.hpp"
#include <chrono>
#include <memory>



class Client : public rclcpp::Node {

    public:

        Client() : Node("Client_node")
            {
            client_ = this->create_client<example_interfaces::srv::AddTwoInts>("nom_canal");
            }

            void sendRequest(long long a, long long b){
                while(!client_->wait_for_service(std::chrono::seconds(1))){
                    if(!rclcpp::ok()){
                        RCLCPP_ERROR(this->get_logger(),"Une erreur s'est produite");
                    }

                    RCLCPP_INFO(this->get_logger(),"Service non disponible nouveau test");
                }
                auto request = std::make_shared<example_interfaces::srv::AddTwoInts::Request>();
                request->a = a;
                request->b = b;

                using Future = rclcpp::Client<example_interfaces::srv::AddTwoInts>::SharedFuture;

                auto callback = [this](Future future){
                    RCLCPP_INFO(this->get_logger(),"Récup de la réponse: %ld",  future.get()->sum);};

                client_->async_send_request(request,callback); // c'est cette fonction qui attend que le lambda en deuxieme param aie un future

            }

       
    private :
        rclcpp::Client<example_interfaces::srv::AddTwoInts>::SharedPtr client_;

};

int main(int argc, char* argv[]){
    rclcpp::init(argc,argv);
    auto node = std::make_shared<Client>();
    node->sendRequest(10,20);
    rclcpp::spin(node);
    rclcpp::shutdown();

    return 0;
    
}
/*
📝 Erreurs sur le Nœud Client

    Portée des types et Templates (Les chevrons) : Tu avais fermé les chevrons de ton pointeur intelligent trop tôt, laissant ::Request à l'extérieur. Le type complet de la donnée doit être dans les chevrons. Il manquait aussi les parenthèses d'instanciation () à la fin du make_shared, et tu avais mis des :: en trop devant les chevrons du template Client<...>::SharedFuture.

    Structure C++ : Tu avais défini la méthode sendRequest à l'intérieur du constructeur Client(). En C++, les méthodes doivent être séparées.

    Héritage : Tu avais écrit public rclcpp::node avec un 'n' minuscule. La classe de base prend toujours une majuscule : rclcpp::Node.

    Logique asynchrone (Le piège du spin) : Dans ton main, tu avais mis rclcpp::spin(node) avant d'appeler sendRequest(). Comme spin() est une boucle infinie qui bloque le programme pour écouter le réseau, ta requête ne partait jamais. Il faut envoyer la requête puis lancer la roue.

    API ROS 2 pour l'attente : Tu as inventé une fonction get_response(). La vraie méthode pour attendre que le serveur soit en ligne est wait_for_service().

    Syntaxe des pointeurs intelligents : Il manquait les parenthèses à la fin de std::make_shared<...::Request>() pour instancier l'objet, et tu avais mis des :: en trop devant les chevrons du template Client<...>::SharedFuture.

    Affichage des logs : Il manquait this->get_logger() comme premier paramètre dans tes RCLCPP_INFO/ERROR. De plus, pour afficher un entier (int64), le bon format est %ld, sans utiliser .c_str() qui est réservé au texte.



*/