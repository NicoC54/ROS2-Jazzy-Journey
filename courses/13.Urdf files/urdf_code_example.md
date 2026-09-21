Pour qu'un programme informatique (comme un nœud de navigation ou de manipulation) puisse contrôler un robot, il doit d'abord connaître son "corps". Il doit savoir de combien de pièces le robot est constitué, comment ces pièces sont attachées entre elles, et quelles sont leurs limites physiques.

C'est exactement le rôle de l'**URDF** (Unified Robot Description Format). C'est un format de fichier standard dans ROS pour décrire la géométrie et la cinématique d'un robot.

### 1. Le langage : XML

L'URDF est écrit en **XML** (eXtensible Markup Language). Le XML est un langage de balisage, c'est-à-dire un texte structuré avec des balises qui s'ouvrent `<balise>` et se ferment `</balise>`. C'est un format arborescent : une balise peut en contenir d'autres. Cette structure en "poupées russes" est parfaite pour décrire un robot, car un bras est attaché à une épaule, elle-même attachée à un torse.

### 2. Le concept central : Links et Joints

La modélisation d'un robot en URDF repose sur une analogie très simple avec le squelette humain : les os et les articulations.

* **Le Link (Lien / Pièce rigide) :** C'est l'équivalent d'un os. Un *link* est une pièce physique du robot qui ne se déforme pas. Dans un bras robotique, l'avant-bras est un link.
* **Le Joint (Articulation) :** C'est ce qui relie exactement deux *links* entre eux. Le coude est un *joint*. Le *joint* définit **comment** les deux pièces peuvent bouger l'une par rapport à l'autre (par exemple, tourner sur un seul axe, ou rester totalement fixes).

### 3. La structure d'un Link

Si on regarde un "os" (link) de plus près dans le code, on ne lui donne pas juste un nom. L'URDF exige que l'on définisse trois aspects physiques pour chaque link :

* **Visual (Visuel) :** C'est ce que l'œil humain ou la caméra verra dans un simulateur (RViz ou Gazebo). On y définit la forme (un cylindre, une boîte, ou un fichier 3D complexe) et la couleur.
* **Collision (Zone de collision) :** C'est la limite physique réelle utilisée par l'ordinateur pour calculer si le robot rentre dans un mur. Souvent, c'est une forme géométrique basique (comme un cylindre englobant) beaucoup plus simple que le modèle visuel, car calculer des collisions sur des formes complexes demande trop de puissance au processeur.
* **Inertial (Inertie) :** C'est la physique de la pièce. On y définit sa masse (en kg) et son centre de gravité. C'est indispensable si tu veux que ton robot soit soumis à la gravité dans un simulateur dynamique.

### 4. La mécanique d'un Joint

Le *joint* est le moteur mathématique de l'URDF. Pour lier deux pièces, l'ordinateur a besoin de règles strictes. Chaque *joint* doit posséder :

* **Un Parent et un Enfant :** La relation est asymétrique. Si l'épaule est le *parent*, le bras est l'*enfant*. Quand le parent bouge, l'enfant suit obligatoirement. Quand l'enfant bouge, le parent ne bouge pas.
* **Un Type :** Est-ce que ça tourne comme une roue (`continuous`), est-ce que ça tourne avec des limites comme un coude (`revolute`), est-ce que ça glisse comme un tiroir (`prismatic`), ou est-ce que c'est soudé (`fixed`) ?
* **Une Origine :** C'est le point d'attache. À quelle distance exacte du parent l'enfant est-il attaché ?
* **Un Axe :** Si ça tourne, autour de quel axe (X, Y ou Z) la rotation s'effectue-t-elle ?

### 5. L'Arbre Cinématique (Kinematic Tree)

En connectant un premier link (souvent appelé `base_link`) à un enfant via un joint, puis cet enfant à un autre enfant via un autre joint, on crée un arbre de dépendances.
C'est la règle d'or de l'URDF standard : **il ne peut pas y avoir de boucles**. Un enfant ne peut avoir qu'un seul parent. L'arbre part d'une racine (`base_link`) et s'étend jusqu'aux extrémités (les roues ou les pinces).

C'est grâce à cet arbre que ROS calcule automatiquement les mathématiques complexes (les matrices de transformation TF2) pour savoir exactement où se trouve le bout de la pince du robot simplement en lisant l'angle de chaque moteur.

---

# Fiche de Cours : Description d'un Robot en URDF

## 1. Définition et Structure Globale

L'**URDF** (Unified Robot Description Format) est un fichier XML définissant la géométrie, l'apparence et la physique d'un robot.
La balise racine est toujours `<robot>`.

```xml
<robot name="mon_premier_robot">
  <!-- Définition des pièces (Links) -->
  <!-- Définition des articulations (Joints) -->
</robot>

```

## 2. Les "Links" (Composants rigides)

Un `<link>` représente un corps rigide. Il contient trois sous-balises optionnelles mais recommandées.

* `<visual>` : Représentation 3D pour l'affichage. Utilise des primitives géométriques (`<box>`, `<cylinder>`, `<sphere>`) ou des maillages (`<mesh filename="..."/>`).
* `<collision>` : Volume de collision pour les algorithmes d'évitement. Doit être aussi simple que possible (primitives géométriques).
* `<inertial>` : Propriétés dynamiques (masse, centre d'inertie `origin`, matrice d'inertie `inertia`).

**Exemple d'un Link (une roue) :**

```xml
<link name="roue_droite">
  <visual>
    <origin xyz="0 0 0" rpy="1.57 0 0"/> <!-- rpy = Roll, Pitch, Yaw -->
    <geometry>
      <cylinder radius="0.1" length="0.05"/>
    </geometry>
    <material name="noir">
      <color rgba="0 0 0 1"/>
    </material>
  </visual>
  <collision>
    <origin xyz="0 0 0" rpy="1.57 0 0"/>
    <geometry>
      <cylinder radius="0.1" length="0.05"/>
    </geometry>
  </collision>
</link>

```

## 3. Les "Joints" (Articulations)

Un `<joint>` connecte exactement deux `<link>` ensemble et définit leur liberté de mouvement.

**Types de joints principaux :**

* `fixed` : Pièces soudées (aucun mouvement).
* `revolute` : Rotation autour d'un axe avec des limites minimales/maximales (ex: coude, genou).
* `continuous` : Rotation sans limite (ex: roue de voiture).
* `prismatic` : Translation le long d'un axe (ex: vérin hydraulique, ascenseur).

**Attributs obligatoires du Joint :**

* `<parent link="nom_du_parent"/>`
* `<child link="nom_de_lenfant"/>`
* `<origin>` : La position (xyz) et l'orientation (rpy) de l'articulation *par rapport au repère du parent*.
* `<axis>` : Le vecteur (x, y, z) autour duquel l'enfant tourne ou translate.
* `<limit>` (pour revolute/prismatic) : Limites d'effort, de vitesse, et de position (lower, upper).

**Exemple d'un Joint (attachant la roue au châssis) :**

```xml
<joint name="joint_chassis_roue_droite" type="continuous">
  <parent link="chassis"/>
  <child link="roue_droite"/>
  <origin xyz="0.2 0.15 0" rpy="0 0 0"/> <!-- Position attachée sur le côté -->
  <axis xyz="0 1 0"/> <!-- Tourne autour de l'axe Y -->
</joint>

```

## 4. Lien avec le Code C++ (ROS 2)

L'URDF est un fichier XML, mais en C++, ton nœud ROS 2 doit pouvoir interagir avec.
En robotique, une tâche courante est de lire l'état réel des moteurs (via l'électronique) et de publier ces angles pour que le robot virtuel (l'URDF) bouge en même temps que le vrai robot. Cela se fait en publiant un message `sensor_msgs::msg::JointState`.

Le programme système `robot_state_publisher` lit l'URDF, écoute tes `JointState`, et calcule toutes les matrices `TF2` automatiquement.

**Exemple C++ : Nœud qui anime une articulation de l'URDF**

```cpp
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include <chrono>
#include <cmath>

using namespace std::chrono_literals;

class JointStatePublisher : public rclcpp::Node {
public:
    JointStatePublisher() : Node("urdf_animator"), angle_(0.0) {
        // Crée un publisher sur le topic "/joint_states"
        publisher_ = this->create_publisher<sensor_msgs::msg::JointState>("joint_states", 10);
        
        // Timer qui s'exécute à 30Hz
        timer_ = this->create_wall_timer(
            33ms, std::bind(&JointStatePublisher::publish_joints, this));
    }

private:
    void publish_joints() {
        auto msg = sensor_msgs::msg::JointState();
        msg.header.stamp = this->get_clock()->now();
        
        // Nom de l'articulation (doit correspondre EXACTEMENT au <joint name="..."> dans l'URDF)
        msg.name.push_back("joint_chassis_roue_droite");
        
        // On simule une rotation continue (la roue tourne)
        angle_ += 0.05;
        msg.position.push_back(angle_);

        publisher_->publish(msg);
    }

    rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    double angle_;
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<JointStatePublisher>());
    rclcpp::shutdown();
    return 0;
}

```

---

À toi de jouer, appliquons la méthode de Feynman. Imagine que je suis un développeur qui n'a jamais touché à la robotique. Explique-moi avec tes propres mots ce qu'est un fichier URDF, à quoi servent les Links et les Joints, et comment la structure en arbre permet à ROS de comprendre le robot. J'attends ton explication pour te corriger ou valider ta compréhension !