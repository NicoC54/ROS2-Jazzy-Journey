// ---------------------------------------------------------
// Exemple complet : Client d'Action en C++ (ROS 2)
// Ce client demande le calcul de la suite de Fibonacci,
// écoute le feedback en direct, et traite le résultat final.
// ---------------------------------------------------------

#include <functional>
#include <memory>
#include <string>
#include <sstream>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "example_interfaces/action/fibonacci.hpp"

class MinimalActionClient : public rclcpp::Node
{
public:
  using Fibonacci = example_interfaces::action::Fibonacci;
  using GoalHandleFibonacci = rclcpp_action::ClientGoalHandle<Fibonacci>;

  MinimalActionClient() : Node("minimal_action_client")
  {
    // Création du client d'action
    client_ptr_ = rclcpp_action::create_client<Fibonacci>(
      this,
      "fibonacci");
  }

  // Fonction pour configurer et envoyer la demande (Goal)
  void send_goal()
  {
    using namespace std::placeholders;

    // 1. On attend que le serveur soit en ligne
    if (!client_ptr_->wait_for_action_server(std::chrono::seconds(10))) {
      RCLCPP_ERROR(this->get_logger(), "Le serveur d'action n'est pas disponible.");
      return;
    }

    // 2. On prépare notre requête (Goal)
    auto goal_msg = Fibonacci::Goal();
    goal_msg.order = 10; // On demande la suite de Fibonacci jusqu'à l'ordre 10

    RCLCPP_INFO(this->get_logger(), "Envoi de l'objectif (Goal) au serveur...");

    // 3. On configure nos 3 Callbacks pour gérer les réponses du serveur
    auto send_goal_options = rclcpp_action::Client<Fibonacci>::SendGoalOptions();
    
    send_goal_options.goal_response_callback =
      std::bind(&MinimalActionClient::goal_response_callback, this, _1);
      
    send_goal_options.feedback_callback =
      std::bind(&MinimalActionClient::feedback_callback, this, _1, _2);
      
    send_goal_options.result_callback =
      std::bind(&MinimalActionClient::result_callback, this, _1);

    // 4. On envoie la demande de manière asynchrone (non-bloquante)
    this->client_ptr_->async_send_goal(goal_msg, send_goal_options);
  }

private:
  rclcpp_action::Client<Fibonacci>::SharedPtr client_ptr_;

  // CALLBACK 1 : Réponse initiale du serveur (Accepté ou Rejeté ?)
  void goal_response_callback(const GoalHandleFibonacci::SharedPtr & goal_handle)
  {
    if (!goal_handle) {
      RCLCPP_ERROR(this->get_logger(), "L'objectif a été REJETÉ par le serveur.");
    } else {
      RCLCPP_INFO(this->get_logger(), "L'objectif a été ACCEPTÉ, exécution en cours...");
    }
  }

  // CALLBACK 2 : Feedback régulier pendant l'exécution
  void feedback_callback(
    GoalHandleFibonacci::SharedPtr,
    const std::shared_ptr<const Fibonacci::Feedback> feedback)
  {
    std::stringstream ss;
    ss << "Feedback reçu (suite partielle) : ";
    for (auto number : feedback->sequence) {
      ss << number << " ";
    }
    // On affiche l'évolution de la suite en temps réel
    RCLCPP_INFO(this->get_logger(), "%s", ss.str().c_str());
  }

  // CALLBACK 3 : Résultat final de l'action
  void result_callback(const GoalHandleFibonacci::WrappedResult & result)
  {
    // On vérifie d'abord comment s'est terminée l'action
    switch (result.code) {
      case rclcpp_action::ResultCode::SUCCEEDED:
        break; // Succès, on continue
      case rclcpp_action::ResultCode::ABORTED:
        RCLCPP_ERROR(this->get_logger(), "L'action a échoué (Abort).");
        return;
      case rclcpp_action::ResultCode::CANCELED:
        RCLCPP_ERROR(this->get_logger(), "L'action a été annulée (Cancel).");
        return;
      default:
        RCLCPP_ERROR(this->get_logger(), "Résultat inconnu.");
        return;
    }

    // Si c'est un succès, on affiche le résultat complet
    std::stringstream ss;
    ss << "Tâche terminée ! Résultat final : ";
    for (auto number : result.result->sequence) {
      ss << number << " ";
    }
    RCLCPP_INFO(this->get_logger(), "%s", ss.str().c_str());
    
    // On coupe le nœud proprement puisque le travail est fini
    rclcpp::shutdown(); 
  }
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  
  // On crée le noeud
  auto action_client = std::make_shared<MinimalActionClient>();
  
  // On lance la fonction qui envoie le Goal
  action_client->send_goal();
  
  // On met le noeud en "spin" pour qu'il puisse écouter le réseau
  // et déclencher les callbacks (feedback, result) quand il reçoit des messages.
  rclcpp::spin(action_client);
  
  rclcpp::shutdown();
  return 0;
}
