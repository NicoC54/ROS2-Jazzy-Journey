Voici ta fiche de révision améliorée. J'y ai intégré les "questions pièges" d'entretien dont nous avons discuté (le faux multi-threading, le retour aux Mutex manuels) et j'ai précisé les définitions techniques pour que tu sois incollable. J'ai également inclus un code C++ complet et corrigé pour illustrer les options.

---

# Fiche Technique : Les Executors et la Concurrence en ROS 2 (C++)

## 1. Définitions Clés

* **Thread (Fil d'exécution) :** Séquence d'instructions exécutées par le processeur. Un programme mono-thread fait une seule chose à la fois.
* **Callback (Fonction de rappel) :** Fonction enregistrée auprès de ROS 2 pour être déclenchée automatiquement à l'arrivée d'un événement (message, timer, service).
* **Executor (Exécuteur) :** Composant architectural de ROS 2 chargé de la boucle d'événements (Event Loop). Il extrait les callbacks prêts de la file d'attente et les soumet aux threads disponibles.

## 2. Typologie des Executors

* **`rclcpp::executors::SingleThreadedExecutor` (Par défaut)**
* Utilise un unique thread.
* Exécution strictement séquentielle des callbacks.
* **Avantages :** Pas de risque de concurrence (thread-safe par nature), pas de corruption de mémoire, facile à déboguer.
* **Inconvénients :** Risque de blocage (starvation). Si un callback est trop long, tous les autres événements attendent.


* **`rclcpp::executors::MultiThreadedExecutor`**
* Utilise un pool de threads configurables (ex: 2, 4, ou selon le CPU).
* Permet l'exécution parallèle des callbacks.
* **Avantages :** Idéal pour les systèmes temps réel (sépare les flux lents des flux rapides).
* **Inconvénients :** Risque de *race conditions* (corruption de données) et de *deadlocks* (interblocages) si mal structuré.



## 3. Gestion de la Concurrence : Les Callback Groups

Pour encadrer le `MultiThreadedExecutor`, on utilise des groupes logiques pour ranger nos fonctions :

* **`MutuallyExclusiveCallbackGroup` :** Empêche l'exécution simultanée des callbacks de **ce même groupe**. Si un thread exécute un callback de ce groupe, les autres callbacks du groupe doivent attendre, même s'il y a des threads inactifs.
* **`ReentrantCallbackGroup` :** Autorise l'exécution simultanée sur différents threads.
* *Précision importante :* Cela permet à différents callbacks du groupe de tourner en même temps, mais cela permet aussi **au même callback de s'exécuter plusieurs fois en parallèle** (ex: si un message Lidar arrive alors que le traitement du précédent n'est pas fini).



### ⚠️ Pièges d'entretien et Limites

1. **Le piège du "Faux Multi-Threading" :** Placer *tous* les callbacks d'un nœud dans un seul et unique `MutuallyExclusiveCallbackGroup` avec un `MultiThreadedExecutor` annule tout parallélisme. Cela revient à recréer un `SingleThreadedExecutor`, mais en consommant plus de ressources.
2. **Le retour aux Mutex manuels (`std::scoped_lock`) :** Les Callback Groups ne gèrent que des règles simples par groupe. Si des règles croisées complexes sont nécessaires (ex: *A et B peuvent tourner en parallèle, mais C est incompatible avec A et B*), les groupes ne suffisent plus. Il faut alors placer les callbacks dans un groupe Réentrant et utiliser manuellement des Mutex C++ à l'intérieur des fonctions pour un contrôle chirurgical.

## 4. Le lien technique : La configuration via les "Options"

En ROS 2, la façon d'associer un Callback Group à une fonction dépend de la nature de cette fonction (Réseau vs Interne).

### Pour les Subscribers (Réseau) : La classe `SubscriptionOptions`
- **Le pattern de l'objet de configuration :** La fonction `create_subscription` possède déjà de nombreux paramètres obligatoires (QoS, topic, callback). Au lieu de la surcharger, ROS 2 regroupe tous les réglages avancés dans un objet unique appelé `SubscriptionOptions`.
- **L'attribut clé :** On utilise `options.callback_group = notre_groupe;` pour associer l'abonnement à notre groupe avant de passer l'ensemble à la fonction.
- *(Note : Le même principe s'applique aux éditeurs avec `PublisherOptions`).*

### ⚠️ Pour les Timers (Interne) : Pas d'objet "Options"
- Contrairement à un Subscriber, un Timer ne communique pas sur le réseau. Il n'a pas besoin de réglages complexes comme les QoS. 
- **Piège à éviter :** On ne peut **pas** passer un `SubscriptionOptions` à un Timer. L'API C++ ne l'accepte pas.
- **La solution :** Pour un Timer, l'API est allégée. On passe directement le groupe en argument de la fonction `create_wall_timer(periode, callback, notre_groupe)`.
## 5. Exemple de Code C++ (Mise en œuvre propre)

```cpp
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

class MonNoeudConcurrent : public rclcpp::Node
{
public:
    MonNoeudConcurrent() : Node("noeud_concurrent")
    {
        // 1. Création de Callback Groups distincts
        groupe_exclusif_ = this->create_callback_group(
            rclcpp::CallbackGroupType::MutuallyExclusive);
        
        groupe_reentrant_ = this->create_callback_group(
            rclcpp::CallbackGroupType::Reentrant);

        // 2. Configuration de l'objet SubscriptionOptions pour le capteur
        rclcpp::SubscriptionOptions options_exclusives;
        options_exclusives.callback_group = groupe_exclusif_;

        // 3. Création du Subscriber (lié au groupe exclusif via les options)
        subscription_capteur_ = this->create_subscription<std_msgs::msg::String>(
            "capteur_donnees", 
            10, // Taille de la file (QoS basique)
            std::bind(&MonNoeudConcurrent::callbackCapteur, this, std::placeholders::_1),
            options_exclusives); // L'étiquette de configuration !

        // 4. Création du Timer (lié au groupe réentrant, passé directement)
        timer_ = this->create_wall_timer(
            500ms,
            std::bind(&MonNoeudConcurrent::callbackTimer, this),
            groupe_reentrant_); // Pour les timers, le groupe passe en 3ème argument
    }

private:
    void callbackCapteur(const std_msgs::msg::String::SharedPtr msg)
    {
        RCLCPP_INFO(this->get_logger(), "Traitement exclusif : %s", msg->data.c_str());
    }

    void callbackTimer()
    {
        RCLCPP_INFO(this->get_logger(), "Traitement timer (réentrant) en parallèle.");
    }

    // Pointeurs intelligents vers nos groupes et objets
    rclcpp::CallbackGroup::SharedPtr groupe_exclusif_;
    rclcpp::CallbackGroup::SharedPtr groupe_reentrant_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_capteur_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<MonNoeudConcurrent>();

    // Instanciation de l'Executor avec 2 threads explicites
    rclcpp::executors::MultiThreadedExecutor executor(
        rclcpp::executors::MultiThreadedExecutorOptions(), 2);
    
    executor.add_node(node);
    
    // Bloque le thread principal et laisse l'Executor gérer les 2 threads de travail
    executor.spin(); 
    
    rclcpp::shutdown();
    return 0;
}

```

---