Dans un robot industriel, les données ne voyagent jamais seules. Un contrôleur de moteur ne se contente pas d'envoyer un simple nombre pour sa vitesse ; il envoie simultanément sa température, sa consommation électrique et son code d'erreur. Les types de messages standards fournis par ROS 2 (comme `std_msgs/Int32` ou `sensor_msgs/Image`) deviennent vite insuffisants pour regrouper ces informations logiquement.

C'est ici qu'interviennent les **Custom Interfaces** (Interfaces personnalisées).

Pour maîtriser ce concept, il faut comprendre un problème fondamental en robotique : l'hétérogénéité des langages. Ton nœud Caméra est peut-être codé en C++ pour la performance, tandis que ton nœud IA est en Python pour utiliser PyTorch. Comment faire comprendre une structure de données complexe à la fois au C++ et au Python ?

**1. L'IDL (Interface Definition Language)**
La solution de ROS 2 est de ne pas écrire la structure de données ni en C++, ni en Python. On l'écrit dans un langage neutre et universel appelé IDL. Un fichier `.msg` n'est rien d'autre qu'un simple fichier texte IDL. Il décrit la donnée de façon abstraite : "Je veux un entier pour l'ID, un flottant pour la température, et un booléen pour l'état".

**2. Le Générateur de Code (`rosidl`)**
C'est la pièce maîtresse du processus. Quand tu compiles ton espace de travail (avec `colcon build`), ROS 2 ne se contente pas de copier ton fichier `.msg`. Il lit ce fichier texte neutre et lance des générateurs automatiques. Ces générateurs vont fabriquer "en coulisse" le code source correspondant : un fichier `.hpp` (struct/class) pour le C++, et un fichier `.py` pour le Python.

**3. Le Package Dédié (La règle d'or architecturale)**
En robotique professionnelle, on ne mélange jamais le code qui définit la structure des données et le code des algorithmes (les nœuds). On crée toujours un package ROS 2 **exclusivement dédié** à la compilation des interfaces (ex: `my_robot_interfaces`). Pourquoi ? Parce que si ton nœud C++ et ton nœud Python veulent utiliser ce message, ils vont tous les deux déclarer le package `my_robot_interfaces` comme dépendance. Si tu avais mis ton `.msg` dans le package de ton algorithme C++, ton nœud Python aurait dû importer tout le code C++ juste pour lire une structure de donnée, ce qui briserait l'architecture.

Voici comment toutes ces idées se matérialisent dans le code.

---

# 📦 Les Custom Interfaces ROS 2 (.msg, .srv, .action)

> **Objectif :** Créer des structures de données sur mesure en utilisant un langage neutre (IDL) et forcer le système de build à générer automatiquement les classes C++ et Python correspondantes dans un package dédié.

## 1. Concepts Fondamentaux

* **Interface ROS 2 :** Un contrat de communication définissant le format exact des données échangées. Il en existe trois types :
* **Message (`.msg`) :** Flux de données continu (ex: télémétrie).
* **Service (`.srv`) :** Requête/Réponse synchrone (ex: "Allume la LED" -> "C'est fait").
* **Action (`.action`) :** Tâche longue avec retours réguliers (ex: "Va à ce point GPS" -> Feedback de distance).


* **IDL (Interface Definition Language) :** Fichier texte neutre décrivant les champs et leurs types de base (`int32`, `float64`, `string`).
* **Génération de code (`rosidl`) :** Processus CMake qui traduit le fichier IDL en structures natives exploitables par le code source des nœuds.

## 2. L'Architecture Obligatoire

Pour créer une Custom Interface, il faut créer un package spécifique avec `ament_cmake` (jamais de package Python pour créer des interfaces).

1. Créer un dossier `msg` à la racine du package.
2. Créer le fichier texte (ex: `MotorStatus.msg`).
3. Modifier le `package.xml` pour ajouter les dépendances de génération.
4. Modifier le `CMakeLists.txt` pour invoquer le générateur automatique de ROS 2.

## 3. Exemples de Code & Configuration

### A. Le fichier neutre : `msg/MotorStatus.msg`

*Note : Le nom du fichier doit toujours commencer par une majuscule (CamelCase).*

```idl
# Constantes (Optionnel, utile pour définir des états)
int32 STATUS_OK=0
int32 STATUS_ERROR=1

# Champs de la structure
int32 motor_id
float64 temperature_celsius
bool is_running
int32 status_code

```

### B. Le `package.xml` du package d'interfaces

Il faut dire à l'outil de build qu'on a besoin du moteur de génération (`build_depend`) et que les nœuds qui utiliseront ce package auront besoin du code généré au moment de l'exécution (`exec_depend`).

```xml
<!-- Outil nécessaire au moment de la compilation pour lire le .msg -->
<build_depend>rosidl_default_generators</build_depend>

<!-- Code généré nécessaire à l'exécution -->
<exec_depend>rosidl_default_runtime</exec_depend>

<!-- Groupe indiquant à ROS 2 que ce package contient des interfaces -->
<member_of_group>rosidl_interface_packages</member_of_group>

```

### C. Le `CMakeLists.txt` du package d'interfaces

C'est ici que la magie opère. La fonction `rosidl_generate_interfaces` va scanner les fichiers et créer les `.hpp`.

```cmake
cmake_minimum_required(VERSION 3.8)
project(my_robot_interfaces)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)

# Demande la génération de code pour notre fichier
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/MotorStatus.msg"
)

ament_package()

```

### D. L'utilisation dans ton Nœud C++ (Un autre package)

Une fois compilé, ROS 2 a généré un fichier `.hpp` dans les dossiers internes de l'ordinateur. Tu peux maintenant l'inclure comme n'importe quelle classe C++.

```cpp
#include "rclcpp/rclcpp.hpp"
// Inclusion du code généré. Remarque le dossier "msg/detail" qui est géré par ROS 2
#include "my_robot_interfaces/msg/motor_status.hpp" 

class MotorNode : public rclcpp::Node {
public:
    MotorNode() : Node("motor_node") {
        publisher_ = this->create_publisher<my_robot_interfaces::msg::MotorStatus>("motor_data", 10);
        
        // Instanciation de l'objet généré (C'est devenu une vraie classe C++)
        auto message = my_robot_interfaces::msg::MotorStatus();
        
        message.motor_id = 42;
        message.temperature_celsius = 65.5;
        message.is_running = true;
        message.status_code = my_robot_interfaces::msg::MotorStatus::STATUS_OK;

        publisher_->publish(message);
    }
private:
    rclcpp::Publisher<my_robot_interfaces::msg::MotorStatus>::SharedPtr publisher_;
};

```

---

Le fichier .md est propre et prêt pour tes notes.

À toi de jouer pour le test de Feynman.
Imagine que je suis un ingénieur mécanique qui commence à coder. Je veux envoyer les données de ma batterie (Voltage, Ampérage, Pourcentage) de mon nœud C++ vers mon Dashboard codé en Python.

Je crée un fichier `batterie_data.cpp` et je mets une classe C++ dedans avec mes variables. Mon code Python ne comprend rien du tout à mon envoi.
Explique-moi physiquement pourquoi mon Python ne peut pas lire ma classe C++, et détaille la chaîne mécanique (de l'IDL jusqu'à CMake) qui va résoudre mon problème. Je t'écoute !