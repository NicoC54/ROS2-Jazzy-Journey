# Cours Complet : Les Actions dans ROS 2

## 1. Introduction : La place des Actions dans l'écosystème ROS

Dans ROS 2, la communication entre les nœuds repose sur trois piliers fondamentaux. Pour bien comprendre les Actions, il faut comprendre ce qu'elles viennent pallier par rapport aux deux autres.

1. **Les Topics (Publishers/Subscribers) :** 
   - *Mécanisme :* Flux de données asynchrone et unidirectionnel.
   - *Usage :* Informations continues (ex: capteurs Lidar, odométrie).
   - *Limite :* Impossible de savoir si le message a été reçu, traité, ou de demander l'exécution d'une tâche précise avec un retour de complétion.
2. **Les Services (Clients/Servers) :** 
   - *Mécanisme :* Requête/Réponse. Le client fait une demande et (généralement) attend la réponse. Même en utilisant des requêtes asynchrones côté programmation, la tâche logique reste une "boîte noire".
   - *Usage :* Calculs rapides, lecture d'un état instantané (ex: allumer une LED, demander un paramètre).
   - *Limite :* Inadapté pour les tâches longues. Pas de retour d'information pendant l'exécution (feedback) et impossibilité d'annuler la requête en cours d'exécution.

**Les Actions** sont la solution de ROS pour les **tâches longues, complexes et potentiellement annulables**. Elles sont en réalité construites en combinant des Topics et des Services sous le capot.

---

## 2. L'Anatomie d'une Action

Une Action établit une communication bidirectionnelle riche entre un **Client d'Action** (celui qui demande) et un **Serveur d'Action** (celui qui exécute). Cette communication repose sur quatre éléments :

1. **Le Goal (Objectif) :** Envoyé par le client. C'est la consigne initiale (ex: "Déplace-toi aux coordonnées X=5, Y=2"). C'est géré comme un Service.
2. **Le Result (Résultat) :** Envoyé par le serveur à la toute fin. C'est l'état final de la tâche (ex: "Succès, coordonnées atteintes" ou "Échec, obstacle infranchissable"). C'est géré comme un Service.
3. **Le Feedback (Retour d'information) :** Envoyé par le serveur *pendant* l'exécution. Il permet au client de suivre l'avancement (ex: "Distance restante : 3 mètres"). C'est géré comme un Topic.
4. **Le Cancel (Annulation/Préemption) :** Envoyé par le client. Demande au serveur d'interrompre la tâche en cours.

### La définition d'une Action (Le fichier `.action`)
Les actions sont définies dans des fichiers portant l'extension `.action`. Ce fichier contient la structure des données échangées, divisée en trois parties séparées par `---`.

*Exemple : `Fibonacci.action`*
```text
# 1. GOAL (Requête du client)
int32 order
---
# 2. RESULT (Réponse finale du serveur)
int32[] sequence
---
# 3. FEEDBACK (Mises à jour intermédiaires)
int32[] partial_sequence
```

---

## 3. La Machine à États du Serveur d'Action

C'est ici que réside la vraie complexité (et puissance) des Actions. Lorsqu'un Serveur d'Action reçoit un Goal, celui-ci ne s'exécute pas magiquement. Il passe par une série d'états stricts (State Machine) que le serveur doit gérer :

1. **Goal Received :** Le client envoie l'objectif. Le serveur doit choisir de l'`Accepter` ou de le `Rejeter` (par exemple, si le robot est déjà occupé ou si les coordonnées sont hors limites).
2. **Executing :** Si accepté, le serveur lance l'exécution de la tâche (souvent dans un thread séparé pour ne pas bloquer le reste de ses communications). Pendant cet état, il publie des **Feedbacks**.
3. **Canceling (Optionnel) :** Si le client envoie une requête d'annulation, le serveur passe dans cet état. Il doit "nettoyer" ce qu'il faisait proprement (ex: freiner le robot) avant de confirmer l'annulation.
4. **États Finaux (Terminaux) :** La tâche se termine toujours par l'un de ces trois états :
   - **Succeeded (Succès) :** La tâche s'est terminée correctement.
   - **Aborted (Échec) :** La tâche a échoué (ex: moteur bloqué).
   - **Canceled (Annulé) :** L'annulation demandée a été correctement traitée.

Le serveur envoie le **Result** final au client au moment de passer dans l'un de ces trois états terminaux.

---

## 4. Architecture de Code : Implémentation

Pour implémenter une Action, le code est divisé par rôles :

### A. Le Client (Celui qui ordonne)
Il doit configurer des fonctions de rappel (Callbacks) pour réagir à 3 événements asynchrones :
- La réponse initiale du serveur (Objectif accepté/rejeté).
- La réception des flux de Feedback.
- La réception du Résultat final.

### B. Le Serveur (Celui qui travaille)
Il doit configurer des Callbacks pour 3 requêtes entrantes :
- **handle_goal :** Accepter ou rejeter le nouvel objectif. (Doit être très rapide, pas de calcul lourd ici).
- **handle_cancel :** Accepter ou rejeter une demande d'annulation.
- **handle_accepted :** Lancer réellement le travail. C'est ici que la tâche longue commence (souvent en lançant une nouvelle routine d'exécution).

Dans la routine d'exécution, le serveur s'assure de publier le feedback régulièrement via `publish_feedback()`, de vérifier si une annulation a été demandée via `is_canceling()`, et de signaler la fin via `succeed()`, `abort()`, ou `canceled()`.

```markdown
# Cours Complet : Les Actions dans ROS 2

## 1. Architecture et Philosophie
Les Actions ROS 2 sont le mécanisme standard pour gérer des tâches asynchrones, longues et complexes (ex: navigation autonome, manipulation d'un bras robotique, traitement d'images lourd). 

Contrairement aux **Topics** (flux unidirectionnel continu) et aux **Services** (requête/réponse instantanée), les Actions offrent un contrôle bidirectionnel complet sur l'exécution d'une tâche dans le temps. Même si un client utilise un appel de Service asynchrone (`async_send_request`) pour éviter de bloquer son propre thread, il reste aveugle à la progression de la tâche et impuissant face à son annulation. L'Action résout cela grâce à la combinaison structurelle du **Feedback** (suivi en temps réel) et de la **Préemption** (interruption contrôlée).

## 2. Anatomie d'une Action (Le fichier `.action`)
Une Action est définie par un contrat strict entre le client et le serveur. Ce contrat est rédigé dans un fichier `.action`, toujours divisé en trois parties séparées par `---`.

* **Le Goal (Objectif) :** Les paramètres initiaux envoyés par le client pour configurer et déclencher la tâche.
* **Le Result (Résultat) :** Les données finales envoyées par le serveur une seule fois, lorsque la tâche est terminée (que ce soit un succès, un échec ou une annulation).
* **Le Feedback (Retour d'état) :** Les données envoyées périodiquement par le serveur pendant l'exécution pour informer le client de la progression (pourcentage accompli, distance restante, etc.).

## 3. La Machine à États du Serveur d'Action
Le serveur d'action ne se contente pas d'exécuter du code de haut en bas ; il gère chaque objectif via une machine à états (State Machine) rigoureuse. 
* **Accepted / Rejected :** Avant même de commencer, le serveur évalue si l'objectif est valide. (Exemple : Les coordonnées demandées sont-elles en dehors de la carte ? Si oui, rejet immédiat).
* **Executing :** La tâche est en cours. C'est dans cet état que le *Feedback* est généré et publié.
* **Canceling :** Le client a demandé une annulation. La tâche ne s'arrête pas brutalement (pour éviter que le robot ne lâche un objet en vol, par exemple). Le serveur passe dans cet état de transition pour nettoyer proprement son processus avant de s'arrêter.
* **Succeeded / Aborted / Canceled :** Les trois états terminaux possibles qui déclenchent l'envoi final du *Result*.

## 4. Fonctionnement sous le capot (ROS 2)
ROS 2 n'invente pas un nouveau protocole réseau complexe pour les Actions. Une Action est en réalité une "méta-structure", une combinaison astucieuse d'outils de base de ROS 2 orchestrée automatiquement en arrière-plan :
1. Un **Service** est utilisé pour envoyer le *Goal* et recevoir l'acceptation/rejet.
2. Un **Service** est utilisé pour récupérer le *Result* final.
3. Un **Service** est utilisé pour transmettre une requête de *Cancel*.
4. Un **Topic** est utilisé pour publier le *Feedback* en continu (le client y est abonné).
5. Un **Topic** est utilisé pour publier le statut global de tous les objectifs en cours sur ce serveur.

```


```cpp
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
    this->client_ptr_ = rclcpp_action::create_client<Fibonacci>(
      this,
      "fibonacci");
  }

  // Fonction pour configurer et envoyer la demande (Goal)
  void send_goal()
  {
    using namespace std::placeholders;

    // 1. On attend que le serveur soit en ligne
    if (!this->client_ptr_->wait_for_action_server(std::chrono::seconds(10))) {
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

```


```cpp
// ---------------------------------------------------------
// Exemple complet : Serveur d'Action en C++ (ROS 2)
// Ce serveur calcule la suite de Fibonacci. Il montre comment 
// gérer l'acceptation, l'annulation et l'exécution en parallèle.
// ---------------------------------------------------------

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

```

Maintenant que tu as l'architecture complète, le serveur et le client, je te redonne la parole pour la méthode de Feynman. Explique-moi la nécessité du Feedback et de l'Annulation pour une tâche complexe (ex: déplacer un objet avec un bras robotique) en utilisant une analogie de tous les jours pour illustrer tes propos.