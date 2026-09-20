# Comprendre la QoS (Quality of Service) dans ROS 2

## 1. Le problème de base : Pourquoi a-t-on inventé la QoS ?
Dans un robot, tu as différents types d'informations qui circulent entre tes programmes (tes nœuds). 
Imaginons deux situations :
*   **Situation A : Un capteur Lidar.** Il tourne très vite et envoie 1000 messages par seconde pour dire où sont les obstacles. Si un message est perdu en cours de route, ce n'est pas grave : un nouveau message, plus récent et plus précis, arrivera une milliseconde plus tard. Essayer de renvoyer le message perdu serait même dangereux, car le robot réagirait à de vieilles données.
*   **Situation B : Un ordre d'arrêt d'urgence.** Un bouton est pressé pour arrêter le robot. Ce message est envoyé une seule fois. S'il est perdu sur le réseau, le robot ne s'arrête pas et s'écrase. Il *doit* arriver, coûte que coûte.

Dans l'ancienne version (ROS 1), tout fonctionnait globalement comme la Situation B (via un protocole réseau strict appelé TCP). C'était très sûr, mais trop lourd et trop lent pour les capteurs modernes. 

Pour résoudre cela, ROS 2 a changé son moteur de communication en utilisant un standard industriel appelé **DDS (Data Distribution Service)**. Le super-pouvoir de DDS, c'est de te permettre de **paramétrer le comportement du réseau pour chaque topic**. C'est ça, la **QoS (Quality of Service)** : un ensemble de règles que tu définis pour dire *comment* tes données doivent voyager.

## 2. Les deux concepts fondamentaux de la QoS

Pour tes entretiens, tu dois maîtriser parfaitement deux "règles" (ou "politiques") de QoS : la **Fiabilité (Reliability)** et la **Durabilité (Durability)**.

### A. La Fiabilité (Reliability)
La fiabilité définit si tu exiges la garantie que ton message arrive à destination.
Il y a deux options principales :
1.  **RELIABLE (Fiable) :** C'est la lettre recommandée avec accusé de réception. L'envoyeur (Publisher) s'assure que le receveur (Subscriber) a bien eu le message. S'il ne l'a pas eu, l'envoyeur le renvoie jusqu'à ce qu'il passe. 
    *   *Usage :* Commandes moteurs, arrêts d'urgence, services.
2.  **BEST_EFFORT (Meilleur effort) :** C'est la carte postale. L'envoyeur met le message dans la boîte aux lettres et l'oublie. S'il se perd, tant pis. Le réseau ne fait aucun effort pour le renvoyer.
    *   *Usage :* Flux vidéo, capteurs Lidar, IMU haute fréquence.

### B. La Durabilité (Durability)
La durabilité définit ce qui arrive aux messages pour les nœuds qui rejoignent la discussion *en retard*.
Imaginons qu'un Publisher envoie une carte (Map) à 10h00. Ton Subscriber s'allume à 10h05. Va-t-il recevoir la carte ?
1.  **VOLATILE (Volatil) :** C'est comme la télévision en direct. Si tu allumes ta télé en retard, tu as raté le début de l'émission. Les messages envoyés avant que le Subscriber ne se connecte sont perdus pour lui.
    *   *Usage :* Données de capteurs en temps réel (on ne veut pas lire un vieux scan laser).
2.  **TRANSIENT_LOCAL (Localement persistant) :** C'est la télévision à la demande (Replay). Le Publisher garde en mémoire les derniers messages qu'il a envoyés. Quand un nouveau Subscriber apparaît, le Publisher lui envoie immédiatement un "résumé" des épisodes précédents (les derniers messages).
    *   *Usage :* La carte d'un environnement (Map), la description géométrique du robot (topic `/robot_description`). Le robot est décrit une fois au démarrage, et tout programme qui s'allume ensuite doit pouvoir lire cette description.

## 3. History et Depth (La mémoire)

Quand on utilise `RELIABLE` ou `TRANSIENT_LOCAL`, le système doit garder des messages en mémoire. La politique **History** définit comment on gère cette mémoire :
*   **KEEP_LAST :** On ne garde que les $N$ derniers messages. 
*   **Depth (Profondeur) :** C'est la valeur de $N$. Par exemple, une `Depth` de 10 signifie qu'on garde une file d'attente des 10 derniers messages. Si un 11ème arrive, le 1er est supprimé.
*   **KEEP_ALL :** On garde tout (très rare, risque de remplir la RAM si personne ne lit les messages).

## 4. La Connexion entre les idées : La Compatibilité (Le point clé de l'entretien)

C'est ici que les problèmes surviennent et que les ingénieurs sont testés.
Un Publisher et un Subscriber ne communiquent ensemble **que si leurs politiques de QoS sont compatibles**.

**La règle d'or :** Le Subscriber ne peut pas exiger un meilleur service que ce que le Publisher est capable d'offrir.

*Exemple sur la Fiabilité (Reliability) :*
*   Publisher `BEST_EFFORT` + Subscriber `BEST_EFFORT` $\rightarrow$ **Ça marche.**
*   Publisher `RELIABLE` + Subscriber `RELIABLE` $\rightarrow$ **Ça marche.**
*   Publisher `RELIABLE` + Subscriber `BEST_EFFORT` $\rightarrow$ **Ça marche.** (Le Publisher est prêt à renvoyer les messages perdus, mais le Subscriber s'en fiche et prend juste ce qui arrive).
*   Publisher `BEST_EFFORT` + Subscriber `RELIABLE` $\rightarrow$ **ERREUR SILENCIEUSE.** (Le Subscriber exige une garantie de livraison, mais le Publisher envoie des cartes postales. Ils ne se parleront jamais, et ROS ne t'affichera pas de message d'erreur évident).

C'est exactement la même matrice pour la Durabilité : un Subscriber `TRANSIENT_LOCAL` (qui veut le replay) ne pourra jamais parler à un Publisher `VOLATILE` (qui fait du direct sans enregistrer).


## 5. Référence complète des méthodes rclcpp::QoS (Pour aller plus loin)
La classe `rclcpp::QoS` en C++ utilise le *method chaining* (on peut enchaîner les appels). Voici toutes les fonctions disponibles pour configurer ton profil :

**Fiabilité & Durabilité :**
*   `.reliable()` : Garantit la livraison.
*   `.best_effort()` : Envoie sans garantie.
*   `.transient_local()` : Garde les messages pour les abonnés tardifs.
*   `.durability_volatile()` : Ne garde rien pour les abonnés tardifs.

**Historique :**
*   `.keep_last(size_t depth)` : Garde seulement les `depth` derniers messages.
*   `.keep_all()` : Garde tous les messages.

**Paramètres Avancés (Bonus Entretien) :**
*   `.deadline(rclcpp::Duration)` : Le temps maximum attendu entre deux messages. Si aucun message n'arrive dans ce délai, un événement (callback) est déclenché. Utile pour détecter si un capteur est tombé en panne.
*   `.lifespan(rclcpp::Duration)` : Durée de vie d'un message. S'il reste bloqué dans la file d'attente plus longtemps que cette durée, il est détruit au lieu d'être lu (évite de traiter des données périmées).
*   `.liveliness(rmw_qos_liveliness_policy_t)` : Définit comment le nœud prouve qu'il est "vivant" au réseau (`AUTOMATIC` par défaut, ou `MANUAL_BY_TOPIC` pour forcer le développeur à le signaler manuellement).
*   `.liveliness_lease_duration(rclcpp::Duration)` : Le temps alloué au nœud pour prouver qu'il est vivant avant d'être considéré comme mort par le système.

## 6. Exemples de code en C++ (ROS 2 Jazzy)

Dans ROS 2 Jazzy, on utilise la classe `rclcpp::QoS` pour configurer tout cela.

### A. Créer un profil de QoS basique (Best Effort pour un Lidar)

```cpp
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"

class LidarNode : public rclcpp::Node {
public:
    LidarNode() : Node("lidar_node") {
        // 1. On crée un objet QoS avec une Depth (historique) de 10
        rclcpp::QoS qos_profile(10);
        
        // 2. On applique la politique de Fiabilité à BEST_EFFORT
        qos_profile.best_effort(); 
        
        // (Optionnel) Par défaut, la durabilité est VOLATILE, 
        // mais on peut le spécifier pour être clair :
        qos_profile.durability_volatile();

        // 3. On crée le publisher en passant le profil QoS
        publisher_ = this->create_publisher<sensor_msgs::msg::LaserScan>(
            "scan", 
            qos_profile // <-- Le profil est passé ici
        );
    }
private:
    rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr publisher_;
};
```

### B. Utiliser les profils prédéfinis de ROS 2

Pour éviter de tout réécrire, ROS 2 fournit des profils tout prêts pour les cas les plus courants. 

```cpp
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "nav_msgs/msg/occupancy_grid.hpp"

class MyNode : public rclcpp::Node {
public:
    MyNode() : Node("my_node") {
        
        // Exemple 1 : Profil par défaut (Reliable, Volatile, Keep_last(10))
        // Utilisé pour 90% de la communication interne.
        auto pub_default = this->create_publisher<std_msgs::msg::String>(
            "chatter", 
            10 // Passer juste un entier (Depth) applique le profil par défaut
        );

        // Exemple 2 : Profil "Sensor Data" (Best Effort, Volatile, Keep_last(5))
        // Parfait pour les caméras, IMU, Lidar.
        auto sub_sensor = this->create_subscription<sensor_msgs::msg::LaserScan>(
            "scan", 
            rclcpp::SensorDataQoS(), // Profil prédéfini !
            std::bind(&MyNode::scan_callback, this, std::placeholders::_1)
        );

        // Exemple 3 : Profil "Transient Local"
        // Très utilisé pour envoyer une map ou la géométrie du robot.
        rclcpp::QoS map_qos(1);
        map_qos.transient_local(); // Le dernier message reste disponible pour les retardataires
        map_qos.reliable();
        
        auto pub_map = this->create_publisher<nav_msgs::msg::OccupancyGrid>(
            "map", 
            map_qos
        );
    }

private:
    void scan_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg) const {
        // Traitement du Lidar
    }
};