# Fiche de Cours Complète : Les Services dans ROS 2 (Jazzy)

## 1. La base absolue : Qu'est-ce que ROS 2 et un Nœud ?

*   **ROS 2 (Robot Operating System)** : Ce n'est pas un système d'exploitation classique (comme Windows ou Linux), mais une "boîte à outils" logicielle (un *middleware*). Elle permet aux différentes parties matérielles et logicielles d'un robot de communiquer entre elles. **Jazzy** (Jazzy Jalisco) est la version stable de 2024 de cette boîte à outils.
*   **Nœud (Node)** : Dans un système robotique complexe, on ne crée pas un seul programme monolithique. On développe une multitude de petits programmes indépendants. Chacun de ces petits programmes est appelé un **Nœud**. 
    *   *Exemple :* Un nœud gère les roues, un autre traite les images de la caméra, et un nœud central décide de la navigation.

## 2. Le besoin de communiquer et la notion de "Service"

Puisque les nœuds sont indépendants, ils ont besoin d'échanger des informations. Si le nœud de navigation veut que le robot prenne une photo, il doit le demander spécifiquement au nœud de la caméra. 

Dans ROS 2, la méthode de communication spécialement conçue pour les actions ponctuelles et garanties est le **Service**. C'est un dialogue fermé entre deux entités, basé sur le modèle **Client/Serveur**.

## 3. Décomposition du modèle Client / Serveur

Pour qu'un Service fonctionne, il met en relation deux rôles distincts :

*   **La Requête (Request)** : C'est la commande envoyée ou la question posée.
*   **La Réponse (Response)** : C'est le résultat de l'action ou la confirmation de son exécution.
*   **Le Client** : C'est le nœud qui a besoin de quelque chose. Il initie la conversation, envoie la *Requête* et se met en attente (souvent de manière asynchrone) de la *Réponse*.
*   **Le Serveur** : C'est le nœud qui fournit le service. Il "écoute" en permanence. Lorsqu'il reçoit une *Requête*, il déclenche une action matérielle ou un calcul, puis construit et renvoie la *Réponse* au Client.

> **Analogie pour retenir :** Au restaurant, tu es le **Client** et tu passes une commande (**Requête**) au cuisinier, qui est le **Serveur**. Le cuisinier prépare le plat et te le fait porter (**Réponse**). Tu attends une réponse précise d'un acteur précis, contrairement à un discours lancé à la foule.

## 4. Quand utiliser un Service ? (Services vs Topics)

Dans ROS 2, la communication la plus basique est le **Topic** (Publish/Subscribe). Un Topic fonctionne comme une diffusion radio : un nœud émet des données en continu (ex: capteur de température), sans se soucier de savoir si quelqu'un écoute. Il n'y a **aucune garantie** de réception.

À l'inverse, le **Service** offre une **fiabilité et une confirmation**. 
On utilise un Service lorsqu'on a besoin de déclencher une action spécifique et d'obtenir la certitude qu'elle a bien été réalisée (ex: "Démarre le laser", "Sauvegarde le fichier").

### Tableau Comparatif
| Caractéristique | Topics (Sujets) | Services |
| :--- | :--- | :--- |
| **Modèle** | Publication / Abonnement (Publish/Subscribe) | Client / Serveur (Request/Response) |
| **Continuité** | Flux continu de données (télémétrie, vidéo) | Action ponctuelle, déclenchée sur demande |
| **Direction** | Unidirectionnel (Publisher -> Subscriber) | Bidirectionnel (Client -> Serveur -> Client) |
| **Garantie** | Aucune garantie de réception ou de traitement | Attente d'une réponse explicite (succès/échec) |

## 5. Comment Client et Serveur se comprennent-ils ? L'Interface (.srv)

Pour dialoguer, les nœuds doivent partager une structure de données stricte, un "contrat". 

*   **L'Interface (fichier `.srv`)** : C'est un fichier texte décrivant exactement les types de données qui circuleront entre le Client et le Serveur.
*   **Structure du fichier** : Un fichier `.srv` est toujours séparé en deux blocs distincts par trois tirets (`---`).
    *   **Au-dessus de `---`** : Ce sont les champs de la **Requête** (ce que le Client envoie). *Exemple : `bool activer_camera`*.
    *   **En dessous de `---`** : Ce sont les champs de la **Réponse** (ce que le Serveur renvoie). *Exemple : `string confirmation_message`*.


# Antisèche : Les Commandes ROS 2 (Jazzy) pour les Services

L'interface en ligne de commande (CLI) de ROS 2 est essentielle pour déboguer, inspecter et interagir avec les services sans avoir à écrire de code. Voici la liste des commandes les plus utiles pour gérer les services.

## 1. Inspecter le réseau

### Lister tous les services actifs
```bash
ros2 service list
```
**Description :** Affiche la liste de tous les services actuellement actifs et disponibles sur le réseau ROS 2. Très utile pour vérifier si ton nœud Serveur a bien démarré.

### Trouver les services utilisant une interface spécifique
```bash
ros2 service find <type_du_service>
```
**Description :** Recherche et liste tous les services actuels qui utilisent un type d'interface `.srv` spécifique.
*Exemple : `ros2 service find std_srvs/srv/Empty`*

## 2. Comprendre les interfaces

### Obtenir le type d'un service
```bash
ros2 service type <nom_du_service>
```
**Description :** Indique le type d'interface exact (le package et le nom du fichier `.srv`) utilisé par le service ciblé. 
*Exemple : `ros2 service type /clear`*

### Voir la structure interne d'une interface (.srv)
```bash
ros2 interface show <type_du_service>
```
**Description :** Dévoile la structure du fichier `.srv`. Indispensable pour voir exactement quelles données envoyer dans la requête (au-dessus des `---`) et ce que la réponse contiendra (en dessous).
*Exemple : `ros2 interface show example_interfaces/srv/AddTwoInts`*

## 3. Interagir et Déboguer

### Appeler un service manuellement depuis le terminal
```bash
ros2 service call <nom_du_service> <type_du_service> "<arguments>"
```
**Description :** Permet d'agir manuellement comme un Client depuis le terminal. Tu envoies une requête au Serveur avec les arguments spécifiés (formatés en YAML) et tu observes la réponse.
*Exemple : `ros2 service call /add_two_ints example_interfaces/srv/AddTwoInts "{a: 2, b: 3}"`*

### Inspecter un nœud spécifique
```bash
ros2 node info <nom_du_noeud>
```
**Description :** Affiche toutes les informations liées à un nœud précis, y compris la liste exhaustive des services qu'il héberge (en tant que Serveur) et des services qu'il contacte (en tant que Client).
Voici la section complète formatée dans un bloc de code Markdown. Tu peux utiliser le bouton "Copier" en haut à droite du cadre pour tout récupérer d'un seul clic et le coller à la fin de ton fichier `.md`.

```markdown
# Implémentation Pratique en C++ (ROS 2 Jazzy)

Pour cet exemple, nous allons utiliser l'interface standard `example_interfaces/srv/AddTwoInts`. 
Rappel de la structure de ce `.srv` :
```text
int64 a
int64 b
---
int64 sum

```

## 1. Le Code du Nœud Serveur

Le serveur déclare le service et définit la **callback** (la fonction qui s'exécute chaque fois qu'une requête est reçue).

```cpp
#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/srv/add_two_ints.hpp"

#include <memory>

class AddTwoIntsServer : public rclcpp::Node
{
public:
  AddTwoIntsServer() : Node("add_two_ints_server")
  {
    // Création du service nommé "add_two_ints"
    // On lie la fonction callback en utilisant std::bind
    service_ = this->create_service<example_interfaces::srv::AddTwoInts>(
      "add_two_ints", 
      std::bind(&AddTwoIntsServer::handle_add_two_ints, this, std::placeholders::_1, std::placeholders::_2)
    );
    RCLCPP_INFO(this->get_logger(), "Prêt à additionner deux entiers (Serveur actif).");
  }

private:
  // La signature de la callback prend toujours :
  // 1. Un pointeur vers la Requête
  // 2. Un pointeur vers la Réponse
  void handle_add_two_ints(
    const std::shared_ptr<example_interfaces::srv::AddTwoInts::Request> request,
    std::shared_ptr<example_interfaces::srv::AddTwoInts::Response> response)
  {
    // On lit les données de la requête et on remplit la réponse
    response->sum = request->a + request->b;
    RCLCPP_INFO(this->get_logger(), "Requête reçue : a=%ld, b=%ld | Réponse envoyée : %ld", 
                request->a, request->b, response->sum);
  }

  // Déclaration du pointeur intelligent gérant le service
  rclcpp::Service<example_interfaces::srv::AddTwoInts>::SharedPtr service_;
};

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  // rclcpp::spin bloque le thread principal et écoute les requêtes entrantes
  rclcpp::spin(std::make_shared<AddTwoIntsServer>());
  rclcpp::shutdown();
  return 0;
}

```

## 2. Le Code du Nœud Client

Le client crée la requête, l'envoie au serveur de manière **asynchrone**, et attend le résultat.

```cpp
#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/srv/add_two_ints.hpp"

#include <chrono>
#include <memory>

using namespace std::chrono_literals;

class AddTwoIntsClient : public rclcpp::Node
{
public:
  AddTwoIntsClient() : Node("add_two_ints_client")
  {
    // Instanciation du client
    client_ = this->create_client<example_interfaces::srv::AddTwoInts>("add_two_ints");
  }

  void send_request(long long a, long long b)
  {
    // 1. Attendre que le serveur soit en ligne
    while (!client_->wait_for_service(1s)) {
      if (!rclcpp::ok()) {
        RCLCPP_ERROR(this->get_logger(), "Interruption système pendant l'attente du service.");
        return;
      }
      RCLCPP_INFO(this->get_logger(), "Serveur non disponible, nouvelle tentative...");
    }

    // 2. Créer la requête et la remplir
    auto request = std::make_shared<example_interfaces::srv::AddTwoInts::Request>();
    request->a = a;
    request->b = b;

    // 3. Envoyer la requête de manière asynchrone
    // On utilise async_send_request pour ne pas bloquer le nœud pendant que le serveur calcule
    using ServiceResponseFuture = rclcpp::Client<example_interfaces::srv::AddTwoInts>::SharedFuture;
    
    // Déclaration d'une fonction lambda pour traiter la réponse quand elle arrivera
    auto response_received_callback = [this](ServiceResponseFuture future) {
      RCLCPP_INFO(this->get_logger(), "Résultat reçu : %ld", future.get()->sum);
    };

    client_->async_send_request(request, response_received_callback);
  }

private:
  rclcpp::Client<example_interfaces::srv::AddTwoInts>::SharedPtr client_;
};

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  
  auto node = std::make_shared<AddTwoIntsClient>();
  node->send_request(41, 1); // Appel de notre méthode personnalisée
  
  // Fait tourner le nœud pour qu'il puisse traiter la callback de réponse
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}

```

## 3. Les Concepts C++ à retenir pour l'entretien

* **`std::shared_ptr` :** Remarque que les requêtes et les réponses ne sont jamais copiées, elles sont passées par pointeurs intelligents. C'est le cœur des performances de ROS 2.
* **`std::bind` vs Lambda :** Dans le serveur, on utilise `std::bind` pour attacher une méthode de classe à la réception du message. Dans le client, on utilise une fonction **lambda** anonyme `[this](ServiceResponseFuture future) { ... }` pour définir ce qu'il se passe au moment où la réponse arrive, ce qui rend le code très lisible.
* **Asynchronisme (`async_send_request`) :** Le client n'utilise jamais d'appel bloquant pur qui gèlerait son thread. Il envoie la requête et passe à autre chose. Quand le serveur répond, l'Executor ROS 2 réveille la callback lambda.

```

```