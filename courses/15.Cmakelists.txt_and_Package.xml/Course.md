# 🏗️ Fondations de Compilation ROS 2 : CMakeLists.txt & package.xml

> **Objectif :** Comprendre l'orchestration de la compilation C++ sous ROS 2 et maîtriser la configuration des outils de build (Colcon/CMake).

## 1. L'architecture de Build
En C++, le code source doit être compilé. ROS 2 utilise un système de build appelé **ament** (sur-couche de CMake) exécuté via l'outil **colcon**. Ce processus est régi par deux fichiers indissociables.

## 2. Le Manifeste : `package.xml`
C'est le document administratif du paquet. Il ne compile rien, il déclare.

### Rôles principaux :
- **Identification :** Fournit les métadonnées (nom, version, licence, auteur).
- **Résolution des dépendances :** Indique au gestionnaire de paquets (comme `rosdep`) quels paquets externes doivent être installés sur la machine cible avant de tenter toute compilation.

### Les balises de dépendances :
- `<buildtool_depend>` : Outils nécessaires pour construire le code (ex: `ament_cmake`).
- `<depend>` : Raccourci moderne dans ROS 2. Déclare une dépendance qui est nécessaire à la fois pour la compilation (Build) et pour l'exécution (Execution). C'est la balise la plus utilisée.

## 3. Le Script de Compilation : `CMakeLists.txt`
C'est le fichier lu par CMake pour transformer les fichiers `.cpp` en binaires exécutables ou en bibliothèques.

### Les 5 étapes obligatoires d'un CMakeLists.txt ROS 2 :

1. **Initialisation**
```cmake
cmake_minimum_required(VERSION 3.8)
project(my_robot_controller)
```
*Définit l'environnement de base.*

2. **Résolution des dépendances (find_package)**
```cmake
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
```
*Localise les bibliothèques sur le disque dur. Le mot-clé `REQUIRED` stoppe la compilation si le paquet est introuvable. Doit correspondre exactement aux balises `<depend>` du `package.xml`.*

3. **Création de la Cible (add_executable)**
```cmake
add_executable(my_node src/my_node.cpp)
```
*Déclare le produit final (`my_node`) et les fichiers sources nécessaires à sa création.*

4. **Édition des Liens (ament_target_dependencies)**
```cmake
ament_target_dependencies(my_node rclcpp std_msgs)
```
*Connecte la cible (`my_node`) aux dépendances trouvées à l'étape 2. C'est ce qui permet au compilateur de lier les définitions de l'API ROS 2 au binaire final.*

5. **Règles d'Installation (install)**
```cmake
install(TARGETS
  my_node
  DESTINATION lib/${PROJECT_NAME}
)
```
*Place le binaire compilé dans le dossier `install/` du workspace ROS 2, permettant à la commande `ros2 run` de le localiser.*

6. **Finalisation**
```cmake
ament_package()
```
*Macro obligatoire qui génère les fichiers de configuration finaux pour ROS 2.*

## 4. Exemples de Code Complets

### Fichier `package.xml`
```xml
<?xml version="1.0"?>
<?xml-model href="[http://download.ros.org/schema/package_format3.xsd](http://download.ros.org/schema/package_format3.xsd)" schematypens="[http://www.w3.org/2001/XMLSchema](http://www.w3.org/2001/XMLSchema)"?>
<package format="3">
  <name>my_robot_controller</name>
  <version>0.0.0</version>
  <description>Contrôleur basique pour robot mobile</description>
  <maintainer email="dev@robotics.com">Ingénieur Junior</maintainer>
  <license>Apache-2.0</license>

  <!-- Outil de build -->
  <buildtool_depend>ament_cmake</buildtool_depend>

  <!-- Dépendances du code C++ -->
  <depend>rclcpp</depend>
  <depend>std_msgs</depend>

  <export>
    <build_type>ament_cmake</build_type>
  </export>
</package>
```

### Fichier `CMakeLists.txt`
```cmake
cmake_minimum_required(VERSION 3.8)
project(my_robot_controller)

# Vérification du compilateur C++
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 1. Trouver les paquets externes
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

# 2. Créer l'exécutable à partir du fichier source
add_executable(my_node src/my_node.cpp)

# 3. Lier l'exécutable avec les dépendances ROS 2
ament_target_dependencies(my_node rclcpp std_msgs)

# 4. Installer l'exécutable dans le workspace
install(TARGETS
  my_node
  DESTINATION lib/${PROJECT_NAME}
)

# 5. Finaliser la configuration du paquet ROS 2
ament_package()
```