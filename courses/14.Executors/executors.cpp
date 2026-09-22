
class Multithread : public rclcpp::Node{

    public:

        Multithread() : Node("Multithread"){

            // on déclare les callbackGroups mutualExclusive et Reentrant
            groupe_reentrant = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);
            groupe_excusif = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

            rclcpp::SubscriptionOptions options_subscriptions;
            options_subscriptions.callback_group = groupe_excusif;


            // on simule un callback subscriber, qui va aller dans un tread callbackgroup exclusif par pur exemple
            subscription_ = this-> create_subscription<std_msgs::msg::String>("nom_canal",10, std::bind(&Multithread::CallbackSubscription, this, std::placeholders::_1), options_subscriptions);

            // on simule un callback timer_ lié à une publication de message, qui va aller dans un thread callbackgroup reentrant par pur exemple
            publisher_ = this -> create_publisher<std_msgs::msg::String>("nom_canal2",10);
            timer_ = this->create_wall_timer(std::chrono::milliseconds(500),std::bind(&Multithread::CallbackTimer,this), groupe_reentrant);
            
            
        }

        //on est obligé de passer un struct SubscriptionOptions en parametre du subscriber contenant le callbackgroup comme attribut. Ce n'est pas le cas pour le timer.
        


        //callback Timer lié à la publication de message
        void CallbackTimer(){

            std_msgs::msg::String msg;
            msg.data = "Création du message";
            publisher_->publish(msg);
        }

       

    private:

        //callback Subscription
        void CallbackSubscription(const std_msgs::msg::String& msg){

            std::cout << msg.data<<std::endl;
        }

        
        //Déclaration des smart pointers utilisés
        std::shared_ptr <rclcpp::CallbackGroup> groupe_reentrant;
        std::shared_ptr <rclcpp::CallbackGroup> groupe_excusif;
        std::shared_ptr <rclcpp::Publisher<std_msgs::msg::String>> publisher_;
        std::shared_ptr <rclcpp::Subscription<std_msgs::msg::String>> subscription_;
        std::shared_ptr <rclcpp::TimerBase> timer_;

};

//main
int main (int argc, char* argv[]){

    //init du node

    rclcpp::init(argc,argv);
    auto node = std::make_shared<Multithread>();

    //appel d'un executor multithreadé initialisé à 3 thread
    rclcpp::executors::MultiThreadedExecutors executor(rclcpp::executors::MultiThreadedExecutorsOptions(),3);
    //ajout du noeud à l'excutor
    executor.add_node(node);
    executor.spin();
    //fermeture propre en cas de Ctrl+c
    rclcpp::shutdown();
    return 0;

    }