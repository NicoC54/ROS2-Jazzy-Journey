# 🚀 ROS 2 Component Nodes & Intra-Process Communication

**Objectif :** Remplacer les nœuds exécutables autonomes par des bibliothèques partagées pour permettre l'exécution de plusieurs nœuds dans un processus unique, réduisant ainsi la latence et la consommation CPU à zéro lors des échanges de données.

## 1. Concepts Fondamentaux

* **Exécutable classique (`add_executable`) :** Crée un processus isolé. Les échanges avec d'autres nœuds nécessitent la copie et la sérialisation des données via la couche réseau middleware (DDS).
* **Component Node (`ament_add_library`) :** Nœud compilé sous forme de module dynamique (`.so`). Il est dépourvu de fonction `main()`.
* **Component Container :** Le processus hôte (fourni par ROS 2) chargé d'héberger en mémoire vive un ou plusieurs Component Nodes.
* **IPC (Intra-Process Communication) :** Optimisation réseau automatique de ROS 2. Si deux nœuds détectent qu'ils sont dans le même conteneur, ils échangent des pointeurs mémoire (Zero-copy) au lieu d'envoyer des messages réseau.

## 2. Les 3 étapes de transformation (Du Nœud au Composant)

1. **Suppression du point d'entrée :** Retirer la fonction `int main(...)` du fichier `.cpp`.
2. **Enregistrement de la classe :** Ajouter une macro spéciale à la toute fin du fichier `.cpp` pour déclarer cette classe comme un composant chargeable dynamiquement.
3. **Modification du CMakeLists.txt :** Changer les instructions de compilation pour fabriquer une bibliothèque partagée plutôt qu'un exécutable, et l'enregistrer dans l'index de ROS 2.

## 3. Exemples de Code C++ & CMake

### Fichier C++ : `my_component.cpp`

```cpp
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

// 1. Définition classique de la classe Nœud
namespace my_namespace {

class MyComponent : public rclcpp::Node {
public:
    // Le constructeur doit obligatoirement accepter des NodeOptions
    MyComponent(const rclcpp::NodeOptions & options) 
    : Node("my_component_node", options) {
        RCLCPP_INFO(this->get_logger(), "Composant chargé en mémoire !");
    }
};

} // namespace my_namespace

// 2. INCLUSION DE LA MACRO D'ENREGISTREMENT (Crucial)
#include "rclcpp_components/register_node_macro.hpp"

// 3. ENREGISTREMENT DU COMPOSANT (Remplace le main)
// Cela permet au "Container" de découvrir cette classe et de l'instancier
RCLCPP_COMPONENTS_REGISTER_NODE(my_namespace::MyComponent)

```

### Fichier CMake : `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.8)
project(my_component_pkg)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED) # Nouvelle dépendance indispensable

# 1. On crée une librairie partagée (SHARED) au lieu d'un exécutable
add_library(my_component SHARED src/my_component.cpp)

# 2. Édition des liens classiques
ament_target_dependencies(my_component rclcpp rclcpp_components)

# 3. Enregistrement auprès du système de composants ROS 2
rclcpp_components_register_node(my_component 
  PLUGIN "my_namespace::MyComponent" 
  EXECUTABLE my_component_node
)

# 4. Installation de la librairie (et non plus de l'exécutable)
install(TARGETS
  my_component
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_package()

```

## 4. Comment lancer un composant dans le terminal ?

Puisqu'il n'y a plus d'exécutable, le `ros2 run` classique ne fonctionne plus.

**Étape A :** Lancer un conteneur vide (le processus hôte)

```bash
ros2 run rclcpp_components component_container

```

**Étape B :** Injecter le composant dans ce conteneur (dans un autre terminal)

```bash
ros2 component load /ComponentManager my_component_pkg my_namespace::MyComponent

```

---

Le rendu Markdown devrait maintenant être parfait dans ton éditeur.

Dès que c'est bon, on reprend notre mise en situation : explique-moi avec tes mots (méthode Feynman) pourquoi un code classique avec `add_executable` fait ramer le processeur quand on transfère des images, et comment le Component Node règle ce problème.