#include <functional>
#include <memory>
#include <thread>
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "example_interfaces/action/fibonacci.hpp"

class MinimalActionServer : public rclcpp::Node
{
public:
  using Fibonacci = example_interfaces::action::Fibonacci;
  using GoalHandleFibonacci = rclcpp_action::ServerGoalHandle<Fibonacci>;

  MinimalActionServer() : Node("minimal_action_server")
  {
    // Création du serveur d'action en liant les 3 Callbacks fondamentaux
    action_server_ = rclcpp_action::create_server<Fibonacci>(
      this,
      "fibonacci",
      std::bind(&MinimalActionServer::handle_goal, this, std::placeholders::_1, std::placeholders::_2),
      std::bind(&MinimalActionServer::handle_cancel, this, std::placeholders::_1),
      std::bind(&MinimalActionServer::handle_accepted, this, std::placeholders::_1));
  }

private:
  rclcpp_action::Server<Fibonacci>::SharedPtr action_server_;

  // CALLBACK 1 : Validation de l'objectif
  // S'exécute quand un client envoie un nouveau Goal.
  rclcpp_action::GoalResponse handle_goal(
    const rclcpp_action::GoalUUID & uuid,
    std::shared_ptr<const Fibonacci::Goal> goal)
  {
    RCLCPP_INFO(this->get_logger(), "Demande reçue pour l'ordre %d", goal->order);
    if (goal->order > 100) {
      // On rejette les demandes irréalistes
      return rclcpp_action::GoalResponse::REJECT; 
    }
    // L'objectif est valide, on l'accepte
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
  }

  // CALLBACK 2 : Gestion de l'annulation
  // S'exécute si le client décide de stopper la tâche en cours.
  rclcpp_action::CancelResponse handle_cancel(
    const std::shared_ptr<GoalHandleFibonacci> goal_handle)
  {
    RCLCPP_INFO(this->get_logger(), "Requête d'annulation reçue du client");
    // On autorise l'annulation (le thread principal s'occupera de s'arrêter proprement)
    return rclcpp_action::CancelResponse::ACCEPT;
  }

  // CALLBACK 3 : Acceptation et lancement
  // S'exécute juste après que handle_goal ait retourné ACCEPT_AND_EXECUTE.
  void handle_accepted(const std::shared_ptr<GoalHandleFibonacci> goal_handle)
  {
    // IMPORTANT : On lance la tâche lourde dans un thread séparé (détaché).
    // Si on bloquait cette fonction, le serveur ne pourrait plus écouter les demandes d'annulation !
    std::thread{std::bind(&MinimalActionServer::execute, this, std::placeholders::_1), goal_handle}.detach();
  }

  // LA TÂCHE PRINCIPALE (Tourne dans son propre thread)
  void execute(const std::shared_ptr<GoalHandleFibonacci> goal_handle)
  {
    rclcpp::Rate loop_rate(1); // Boucle à 1 Hz (1 seconde de pause par itération)
    const auto goal = goal_handle->get_goal();
    auto feedback = std::make_shared<Fibonacci::Feedback>();
    auto result = std::make_shared<Fibonacci::Result>();
    
    auto & sequence = feedback->sequence;
    sequence.push_back(0);
    sequence.push_back(1);

    for (int i = 1; (i < goal->order) && rclcpp::ok(); ++i) {
      // VÉRIFICATION CONSTANTE : Le client a-t-il demandé une annulation ?
      if (goal_handle->is_canceling()) {
        result->sequence = sequence;
        goal_handle->canceled(result); // On prévient ROS que la tâche est officiellement annulée
        RCLCPP_INFO(this->get_logger(), "Exécution annulée proprement.");
        return; // On stoppe la boucle
      }
      
      // EXÉCUTION & FEEDBACK : On calcule et on informe le client
      sequence.push_back(sequence[i] + sequence[i - 1]);
      goal_handle->publish_feedback(feedback);
      RCLCPP_INFO(this->get_logger(), "Calcul en cours... Feedback publié.");
      
      loop_rate.sleep(); // On simule le fait que la tâche prend du temps
    }

    // SUCCÈS : Si on sort de la boucle sans annulation, c'est gagné
    if (rclcpp::ok()) {
      result->sequence = sequence;
      goal_handle->succeed(result); // On envoie le Result final au client
      RCLCPP_INFO(this->get_logger(), "Tâche terminée avec succès !");
    }
  }
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MinimalActionServer>());
  rclcpp::shutdown();
  return 0;
}