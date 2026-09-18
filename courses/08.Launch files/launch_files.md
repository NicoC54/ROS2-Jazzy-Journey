# Fiche de Révision : Les Launch Files sous ROS 2 (Jazzy)

## 1. Partie Cours : Concepts et Définitions

### Rôle et Définition

Dans un système robotique complexe, démarrer chaque nœud (processus) manuellement dans des terminaux séparés est inefficace.
Le Launch File est un script d'orchestration (généralement en Python sous ROS 2) qui automatise :

* Le démarrage simultané de plusieurs nœuds.
* La configuration de leurs paramètres.
* Le renommage dynamique pour éviter les conflits.

**Commande clé :** `ros2 launch <nom_du_package> <nom_du_launch_file.py>`

### Le Mécanisme (Concept de la `LaunchDescription`)

Un script de launch file Python ne démarre pas les processus lui-même.
Son rôle est de construire une liste d'instructions appelée `LaunchDescription`.
Une fois que le script a fini de lister les actions (ex: "préparer le nœud A", "déclarer l'argument B"), le gestionnaire de lancement ROS 2 lit cette `LaunchDescription` finale et exécute les actions en arrière-plan.

### L'Action Node

Pour intégrer un nœud à la description, on utilise l'objet `Node`. Il nécessite au minimum trois attributs fondamentaux :

* **`package`** : Le dossier contenant le code.
* **`executable`** : Le fichier compilé ou le script contenant les instructions du processus.
* **`name`** : Le nom assigné au nœud lors de l'exécution (crucial pour instancier plusieurs fois le même exécutable sans conflit).

### Modularité : Arguments vs Paramètres

* **Launch Argument (Argument de lancement)** : Variable passée par l'utilisateur dans le terminal lors de l'appel du launch file (ex: `vitesse:=5`). Apporte de la versatilité sans recompiler.
* **Parameter (Paramètre)** : Variable interne consommée par le nœud (le code C++ ou Python du processus).
* **La connexion** : Le Launch File utilise l'objet `LaunchConfiguration` pour capturer l'Argument de lancement et l'injecter dynamiquement dans le Paramètre du nœud.

---

## 2. Partie Pratique : Exemple de Launch File (Python)

Voici un exemple classique d'un launch file qui démarre un capteur et un contrôleur de moteurs, tout en permettant à l'utilisateur de modifier la vitesse depuis le terminal.

```python
# Importation des modules nécessaires de ROS 2
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    """
    Cette fonction est le point d'entrée obligatoire pour ROS 2.
    Elle doit retourner un objet LaunchDescription.
    """

    # 1. Déclaration d'un Argument de lancement (externe)
    # L'utilisateur pourra taper : ros2 launch mon_pkg mon_launch.py vitesse_max:=3.5
    vitesse_arg = DeclareLaunchArgument(
        'vitesse_max',
        default_value='2.0', # Valeur par défaut si l'utilisateur ne précise rien
        description='Vitesse maximale du robot pour le test'
    )

    # 2. Configuration du premier nœud (un capteur lidar)
    capteur_node = Node(
        package='mon_package_robot',
        executable='capteur_laser_exec',
        name='lidar_avant' # Nom spécifique pour éviter les conflits
    )

    # 3. Configuration du second nœud (un contrôleur de moteur)
    controleur_node = Node(
        package='mon_package_robot',
        executable='controleur_moteur_exec',
        name='controleur_principal',
        parameters=[
            # On injecte l'argument "vitesse_max" dans le paramètre interne "vitesse_limite"
            {'vitesse_limite': LaunchConfiguration('vitesse_max')}
        ]
    )

    # 4. Assemblage et retour du plan d'action
    # On ajoute toutes les entités créées dans la LaunchDescription
    return LaunchDescription([
        vitesse_arg,
        capteur_node,
        controleur_node
    ])

```