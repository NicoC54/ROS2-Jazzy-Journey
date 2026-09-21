Voici la version finale, complète et restructurée de ta fiche technique. Elle rassemble absolument tous les concepts, les pièges d'entretien, les analogies pour bien mémoriser, et le code C++ parfait.

Tu peux copier-coller ce bloc entier dans ton outil de prise de notes (Notion, Obsidian, Markdown...) !

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

## 4. 🌟 Zoom : Le secret du vrai Multi-Threading (Multiplier les boîtes exclusives)

Il est fondamental de comprendre que l'on peut (et que l'on doit) créer **plusieurs** `MutuallyExclusiveCallbackGroup` au sein d'un même nœud.

* **La règle :** Les fonctions d'une *même* boîte exclusive ne peuvent pas tourner en même temps.
* **La magie :** Les fonctions de la *Boîte A* **peuvent** s'exécuter en même temps que les fonctions de la *Boîte B* !

> **L'analogie de la cuisine (2 Chefs = 2 Threads) :**
> * **Boîte 1 (Le Four) :** Un seul chef peut utiliser le four à la fois.
> * **Boîte 2 (Le Mixeur) :** Un seul chef peut utiliser le mixeur à la fois.
> * **Résultat :** Le Chef A peut utiliser le mixeur *exactement en même temps* que le Chef B utilise le four. C'est du parallélisme sécurisé !
> 
> 

## 5. Architecture : Comment choisir son Callback Group ? (Matrice de Décision)

En entretien, face à une architecture, on se pose 3 questions pour classer un callback :

1. **Modifie-t-il une variable partagée de la classe ?** (ex: `this->position = ...`)
* **OUI :** Danger de corruption. 👉 `MutuallyExclusiveCallbackGroup`.
* **NON :** Calcul pur (stateless). 👉 `ReentrantCallbackGroup`.


2. **Communique-t-il directement avec du matériel physique ?** (ex: bus CAN, port USB)
* **OUI :** Pour éviter d'envoyer des signaux brouillés en simultané. 👉 `MutuallyExclusiveCallbackGroup`.


3. **Fait-il un appel synchrone à un Service ROS 2 ?** (Le piège du Deadlock)
* **OUI :** Si un Timer appelle un Service et attend la réponse dans un groupe exclusif, la réponse du service ne pourra pas entrer dans ce même groupe verrouillé !
* 👉 **Choix obligatoire :** Mettre le Timer et la réponse du Service dans des groupes *différents*, ou utiliser un `ReentrantCallbackGroup`.



> **Règle d'or de l'architecte :** Par défaut, créez **plusieurs** `MutuallyExclusiveCallbackGroups` séparés par sous-systèmes (ex: 1 boîte pour la vision, 1 boîte pour les moteurs). N'utilisez le `Reentrant` que si c'est strictement nécessaire pour la performance de fonctions indépendantes.

## 6. Le lien technique : La configuration via les "Options"

En ROS 2, la façon d'associer un Callback Group dépend de la nature de la fonction (Réseau vs Interne).

* **Pour les Subscribers (Réseau) : La classe `SubscriptionOptions**`
* **Le pattern de l'objet de configuration :** La fonction `create_subscription` possède déjà de nombreux paramètres obligatoires (QoS, topic, callback). Au lieu de la surcharger, ROS 2 regroupe tous les réglages avancés dans un objet unique appelé `SubscriptionOptions`.
* **L'attribut clé :** On utilise `options.callback_group = notre_groupe;` pour associer l'abonnement à notre groupe avant de passer l'ensemble à la fonction.


* **⚠️ Pour les Timers (Interne) : Pas d'objet "Options"**
* Contrairement à un Subscriber, un Timer ne communique pas sur le réseau. Il n'a pas besoin de réglages complexes comme les QoS.
* **Piège à éviter :** On ne peut **pas** passer un `SubscriptionOptions` à un Timer.
* **La solution :** Pour un Timer, l'API est allégée. On passe directement le groupe en argument de la fonction `create_wall_timer(periode, callback, notre_groupe)`.



## 7. L'exception du Publisher : Action vs Réaction

L'Executor gère les **réactions** (les callbacks asynchrones comme les Timers et Subscribers).

* Un **Publisher** est une **action** synchrone (un simple "mégaphone"). Il ne possède pas de callback pour l'envoi de données.
* **Conclusion :** On n'associe pas l'action de publier à un Callback Group. Le Publisher attend sagement que l'on appelle sa méthode `publish()` **à l'intérieur** d'une fonction callback (ex: dans la fonction d'un Timer rangé dans son propre groupe).

## 8. Exemple de Code C++ (Mise en œuvre propre)

```cpp
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "geometry_msgs/msg/twist.hpp"

using namespace std::chrono_literals;

class MonNoeudConcurrent : public rclcpp::Node
{
public:
    MonNoeudConcurrent() : Node("noeud_concurrent")
    {
        // 1. Création de MULTIPLES Callback Groups distincts
        groupe_vision_ = this->create_callback_group(
            rclcpp::CallbackGroupType::MutuallyExclusive);
            
        groupe_moteurs_ = this->create_callback_group(
            rclcpp::CallbackGroupType::MutuallyExclusive);
        
        groupe_reentrant_ = this->create_callback_group(
            rclcpp::CallbackGroupType::Reentrant);

        // 2. Configuration des objets SubscriptionOptions
        rclcpp::SubscriptionOptions options_vision;
        options_vision.callback_group = groupe_vision_;

        rclcpp::SubscriptionOptions options_moteurs;
        options_moteurs.callback_group = groupe_moteurs_;

        // 3. Création des Subscribers (liés à des groupes exclusifs DIFFÉRENTS)
        // La vision et les moteurs tourneront en parallèle sans jamais se bloquer !
        sub_camera_ = this->create_subscription<sensor_msgs::msg::Image>(
            "camera/image", 10, 
            std::bind(&MonNoeudConcurrent::cb_vision, this, std::placeholders::_1), 
            options_vision);
            
        sub_cmd_vel_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "cmd_vel", 10, 
            std::bind(&MonNoeudConcurrent::cb_moteurs, this, std::placeholders::_1), 
            options_moteurs);

        // 4. Création du Timer (lié au groupe réentrant, groupe passé directement)
        timer_ = this->create_wall_timer(
            500ms, std::bind(&MonNoeudConcurrent::cb_timer, this), groupe_reentrant_);
            
        // 5. Création du Publisher (Pas de Callback Group, c'est une action !)
        publisher_ = this->create_publisher<std_msgs::msg::String>("topic_sortie", 10);
    }

private:
    void cb_vision(const sensor_msgs::msg::Image::SharedPtr msg)
    {
        RCLCPP_INFO(this->get_logger(), "Traitement lourd de l'image (Exclusif Vision)");
    }
    
    void cb_moteurs(const geometry_msgs::msg::Twist::SharedPtr msg)
    {
        RCLCPP_INFO(this->get_logger(), "Commande envoyée aux roues (Exclusif Moteurs)");
    }

    void cb_timer()
    {
        RCLCPP_INFO(this->get_logger(), "Traitement timer (réentrant) en parallèle.");
        // Le Publisher est utilisé ICI, à l'intérieur de l'événement Timer
        auto msg = std_msgs::msg::String();
        msg.data = "Tick Timer";
        publisher_->publish(msg);
    }

    // Pointeurs intelligents vers nos groupes et objets
    rclcpp::CallbackGroup::SharedPtr groupe_vision_;
    rclcpp::CallbackGroup::SharedPtr groupe_moteurs_;
    rclcpp::CallbackGroup::SharedPtr groupe_reentrant_;
    
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr sub_camera_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr sub_cmd_vel_;
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<MonNoeudConcurrent>();

    // Instanciation de l'Executor avec 3 threads explicites (1 vision, 1 moteur, 1 timer)
    rclcpp::executors::MultiThreadedExecutor executor(
        rclcpp::executors::MultiThreadedExecutorOptions(), 3);
    
    executor.add_node(node);
    
    // Bloque le thread principal et laisse l'Executor gérer les threads de travail
    executor.spin(); 
    
    rclcpp::shutdown();
    return 0;
}

```

---