Toutes mes excuses, j'avais mal compris votre demande précédente !

Voici la fiche de révision complète et fusionnée, qui ignore la partie WSL et commence exactement à la configuration de ROS 2, en allant jusqu'aux actions.

Vous pouvez tout copier d'un coup dans votre fichier `.md` :

```markdown
# Fiche de Révision ROS 2 : Les Commandes Fondamentales
*Basé sur le cours "Learn ROS 2: Beginner to Advanced" par Kevin Wood*

## 1. Configuration de ROS 2 (Humble / Jazzy)
Votre terminal ne reconnaît pas nativement les commandes `ros2`. Il faut lui indiquer où elles se trouvent en "sourçant" (sourcing) l'installation.

* **Sourcer manuellement ROS 2 dans le terminal actuel** :
  ```bash
  source /opt/ros/humble/setup.bash

```

*(Remplacez `humble` par `jazzy` selon votre version).*

* **Sourcer ROS 2 automatiquement à chaque nouveau terminal** :
```bash
echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc

```



---

## 2. Gestion des Paquets (Packages)

Dans ROS 2, le code est organisé en **Packages** (paquets). Un paquet contient des exécutables (les programmes que vous pouvez lancer) et d'autres fichiers de configuration.

* **Lister tous les paquets ROS 2 installés sur le système** :
```bash
ros2 pkg list

```


* **Lister tous les exécutables de tous les paquets** :
```bash
ros2 pkg executables

```


* **Lister uniquement les exécutables d'un paquet spécifique** (ex: *turtlesim*) :
```bash
ros2 pkg executables turtlesim

```



---

## 3. Exécution de programmes (Exécutables)

Pour lancer un programme, on utilise la commande `ros2 run` suivie du nom du paquet et du nom de l'exécutable.

* **Syntaxe générale** :
```bash
ros2 run <nom_du_paquet> <nom_de_l_executable>

```


* **Exemple 1 : Lancer le simulateur de la tortue** :
```bash
ros2 run turtlesim turtlesim_node

```


* **Exemple 2 : Lancer le programme pour contrôler la tortue au clavier** (dans un *deuxième* terminal) :
```bash
ros2 run turtlesim turtle_teleop_key

```



---

