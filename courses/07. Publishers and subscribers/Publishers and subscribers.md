# Fiche de Cours Complète : Publishers et Subscribers en C++ dans ROS 2 (Jazzy)

## Partie 1 : Le Cours Détaillé

### 1. Introduction : La logique orientée objet (C++)
En ROS 2 avec C++, on utilise la **Programmation Orientée Objet (POO)**. Chaque Nœud que tu crées va être une "Classe" qui hérite des capacités de base d'un nœud ROS 2 (via `rclcpp::Node`). 
Au lieu d'écrire un script linéaire qui s'exécute de haut en bas, on écrit un programme qui réagit à des événements : "Toutes les secondes, fais ceci" ou "Quand tu reçois un message, fais cela".

### 2. Définitions des concepts techniques
*   **`rclcpp` (ROS Client Library for C++)** : C'est la bibliothèque de base. Elle contient tout le code source de ROS 2 pour le C++.
*   **Callback (Fonction de rappel)** : C'est une fonction qui est "mise en attente" et se déclenche automatiquement *uniquement* quand un événement précis se produit (minuteur qui sonne, message reçu).
*   **Timer (Minuteur)** : Un objet qui déclenche un Callback à intervalle régulier.
*   **QoS (Quality of Service)** : La taille de la "file d'attente" (souvent `10`). Si les messages arrivent trop vite, le nœud en garde 10 en mémoire et jette les plus anciens.

---

### 3. Le Publisher (Éditeur) en C++ : Explication Ligne par Ligne

Voici le code complet d'un nœud qui publie un message toutes les demi-secondes.

```cpp
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include <chrono>

using namespace std::chrono_literals;

class MonPublisher : public rclcpp::Node 
{
public:
    MonPublisher() : Node("noeud_bavard"), compte_(0) 
    {
        publisher_ = this->create_publisher<std_msgs::msg::String>("mon_topic_texte", 10);
        timer_ = this->create_wall_timer(500ms, std::bind(&MonPublisher::timer_callback, this));
    }

private:
    void timer_callback() 
    {
        auto message = std_msgs::msg::String();
        message.data = "Bonjour ROS 2 ! N° " + std::to_string(compte_++);
        RCLCPP_INFO(this->get_logger(), "Je publie : '%s'", message.data.c_str());
        publisher_->publish(message);
    }

    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    size_t compte_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MonPublisher>());
    rclcpp::shutdown();
    return 0;
}
```

**Décryptage du professeur :**

*   `#include "rclcpp/rclcpp.hpp"` : On importe le dictionnaire global de ROS 2 pour le C++. Sans ça, le compilateur ne sait pas ce qu'est un "Node".
*   `#include "std_msgs/msg/string.hpp"` : On importe la structure exacte du message que l'on va envoyer (ici, une simple chaîne de caractères).
*   `using namespace std::chrono_literals;` : Une astuce qui nous permet d'écrire `500ms` directement, au lieu d'une longue formule mathématique pour gérer le temps.
*   `class MonPublisher : public rclcpp::Node` : On crée notre propre modèle (`MonPublisher`). Le mot-clé `public` signifie qu'on "hérite" de toutes les capacités d'un nœud ROS 2 standard.
*   `MonPublisher() : Node("noeud_bavard"), compte_(0)` : C'est le **Constructeur**. Il s'exécute une seule fois à la naissance de l'objet. On baptise notre nœud `"noeud_bavard"` sur le réseau, et on initialise notre variable `compte_` à 0.
*   `publisher_ = this->create_publisher<std_msgs::msg::String>("mon_topic_texte", 10);` : `this->` fait référence à notre nœud. On lui dit de créer un éditeur. Entre `< >`, on précise le type de message. Ensuite, on donne le nom du canal (`"mon_topic_texte"`) et la QoS (file d'attente de `10`).
*   `timer_ = this->create_wall_timer(...)` : On crée le minuteur. `500ms` est le rythme. `std::bind` est une fonction C++ complexe mais essentielle : elle sert à attacher (lier) le minuteur à notre fonction `timer_callback`.
*   `void timer_callback()` : C'est l'action qui sera exécutée à chaque battement du minuteur.
*   `auto message = std_msgs::msg::String();` : On crée une boîte vide ayant la forme de notre message.
*   `message.data = ...` : On remplit la case `data` du message. `std::to_string` transforme notre chiffre en texte pour l'ajouter à la phrase.
*   `RCLCPP_INFO(...)` : C'est l'équivalent du `printf` ou `cout`. Ça affiche un texte dans le terminal, mais en l'intégrant au système de journalisation (logs) de ROS 2.
*   `publisher_->publish(message);` : L'action finale ! On balance la boîte sur le réseau.
*   *Section `private:`* : On déclare nos variables globales pour cette classe (le timer, le publisher, et le compteur) pour que le nœud s'en souvienne entre chaque battement.
*   *La fonction `main`* : 
    *   `rclcpp::init(argc, argv);` : Allume le moteur ROS 2.
    *   `rclcpp::spin(std::make_shared<MonPublisher>());` : Crée notre nœud et le fait "tourner" (spin) à l'infini pour qu'il ne s'éteigne pas.
    *   `rclcpp::shutdown();` : Éteint le moteur proprement si on fait un Ctrl+C.

---

### 4. Le Subscriber (Abonné) en C++ : Explication Ligne par Ligne

Le Subscriber n'a pas de minuteur. Il est passif et réagit à la réception d'un message.

```cpp
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class MonSubscriber : public rclcpp::Node 
{
public:
    MonSubscriber() : Node("noeud_ecouteur") 
    {
        subscriber_ = this->create_subscription<std_msgs::msg::String>(
            "mon_topic_texte", 10, 
            std::bind(&MonSubscriber::topic_callback, this, std::placeholders::_1));
    }

private:
    void topic_callback(const std_msgs::msg::String & msg) const 
    {
        RCLCPP_INFO(this->get_logger(), "J'ai entendu : '%s'", msg.data.c_str());
    }

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscriber_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MonSubscriber>());
    rclcpp::shutdown();
    return 0;
}
```

**Décryptage du professeur :**

*   `MonSubscriber() : Node("noeud_ecouteur")` : Le constructeur baptise ce nœud `"noeud_ecouteur"`.
*   `subscriber_ = this->create_subscription<std_msgs::msg::String>(...)` : On demande au nœud de créer un abonnement. On lui précise le type attendu (`<std_msgs::msg::String>`), le nom du topic à écouter (`"mon_topic_texte"`), et la QoS (`10`).
*   `std::bind(&MonSubscriber::topic_callback, this, std::placeholders::_1)` : On lie la réception d'un message à notre fonction `topic_callback`. Le `std::placeholders::_1` est crucial : il signifie "prends le message qui vient d'arriver, et envoie-le comme 1er paramètre à la fonction callback".
*   `void topic_callback(const std_msgs::msg::String & msg) const` : Voici la fonction de réaction. Elle reçoit `msg` en paramètre. Le `const` garantit qu'on ne va pas modifier le message par accident en le lisant, c'est une sécurité.
*   `RCLCPP_INFO(...)` : On affiche simplement le contenu (`msg.data`) dans le terminal.
*   `rclcpp::Subscription<...>::SharedPtr subscriber_;` : La déclaration de notre objet subscriber pour qu'il reste en vie tant que la classe existe.

---

## Partie 2 : Les Commandes Utiles (CLI)

### 1. Lancer et observer les nœuds
*   **Lancer le Publisher :**
```bash
ros2 run <nom_du_package> mon_publisher
```
*   **Lancer le Subscriber (dans un autre terminal) :**
```bash
ros2 run <nom_du_package> mon_subscriber
```

### 2. Déboguer les Topics
*   **Voir les données brutes circuler en temps réel :**
```bash
ros2 topic echo /mon_topic_texte
```
*   **Vérifier la fréquence d'envoi du Publisher :**
```bash
ros2 topic hz /mon_topic_texte
```
*   **Créer un "Faux" Publisher depuis le terminal (pour tester ton Subscriber en C++) :**
```bash
ros2 topic pub /mon_topic_texte std_msgs/msg/String "{data: 'Test depuis le terminal'}"