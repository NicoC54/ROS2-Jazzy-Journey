# Fiche de Cours Complète : Les Paramètres dans ROS 2 (Jazzy)

## Partie 1 : Le Cours Détaillé

### 1. Introduction : L'analogie du jeu vidéo
Imagine que tu joues à un jeu vidéo. Avant ou pendant la partie, tu vas dans le menu "Options" pour régler le volume de la musique, la sensibilité de la souris ou la difficulté (Facile, Normal, Difficile). 
Dans ROS 2, c'est exactement la même chose pour tes Nœuds : tu ne veux pas recréer un nouveau nœud à chaque fois que tu dois modifier un petit détail de son comportement. Tu vas utiliser les **Paramètres**.

### 2. Qu'est-ce qu'un Paramètre ?
**Définition :** Un Paramètre est une valeur de configuration associée à un Nœud spécifique. C'est un réglage que l'on peut lire ou modifier pour changer le comportement du programme.

*   **Les Types de données :** Comme pour les variables en programmation, chaque paramètre a un "type" strict. Les plus courants sont :
    *   `integer` (nombre entier, ex: 5, -10)
    *   `double` (nombre à virgule, ex: 3.14)
    *   `boolean` (vrai ou faux, ex: True, False)
    *   `string` (texte, ex: "camera_frontale")
    *   `list` (tableau contenant plusieurs valeurs du même type)

### 3. Comment fonctionnent les Paramètres ?
Voici les trois concepts clés pour comprendre comment les idées se connectent dans l'écosystème ROS 2 :

*   **L'appartenance locale (Scope) :** Les paramètres **ne sont pas globaux**. Il n'y a pas un "grand tableau de paramètres" pour tout le robot. **Chaque nœud possède son propre dictionnaire de paramètres**. Si le Nœud `camera` a un paramètre `luminosité`, ce paramètre appartient uniquement à la caméra.
*   **La reconfiguration dynamique :** C'est la plus grande force des paramètres dans ROS 2. Tu n'as pas besoin de fermer ton Nœud, de recompiler ton code et de le relancer pour changer une valeur. Tu peux modifier un paramètre *pendant* que le Nœud est en cours d'exécution, et le Nœud s'adaptera instantanément (ex: limiter la vitesse max du robot en plein mouvement).
*   **La sauvegarde (Les fichiers YAML) :** Si tu passes 2 heures à trouver les réglages parfaits pour les moteurs de ton robot, tu ne veux pas avoir à les retaper à chaque démarrage. ROS 2 permet de "sauvegarder" (dump) tous les paramètres d'un nœud dans un fichier texte structuré appelé un fichier **YAML**. Tu pourras ensuite demander à ton nœud de "charger" ce fichier à son prochain démarrage.

---

## Partie 2 : Les Commandes Utiles (CLI)

L'outil en ligne de commande de ROS 2 permet de manipuler ces réglages très facilement.

### 1. Explorer les Paramètres
* **Lister tous les paramètres disponibles de tous les nœuds actifs :**
```bash
ros2 param list
```

* **Obtenir la description et le type d'un paramètre spécifique :**
```bash
ros2 param describe /<nom_du_noeud> <nom_du_parametre>
```
*(C'est très utile pour savoir si tu dois entrer un texte, un entier ou un booléen).*

### 2. Lire et Modifier (Get & Set)
* **Lire la valeur actuelle d'un paramètre :**
```bash
ros2 param get /<nom_du_noeud> <nom_du_parametre>
```

* **Modifier (écrire) la valeur d'un paramètre en temps réel :**
```bash
ros2 param set /<nom_du_noeud> <nom_du_parametre> <nouvelle_valeur>
```
*(Exemple : `ros2 param set /turtlesim background_r 150` change la couleur de fond instantanément).*

### 3. Sauvegarder et Charger (Fichiers YAML)
* **Sauvegarder (exporter) tous les paramètres actuels d'un nœud dans un fichier :**
```bash
ros2 param dump /<nom_du_noeud>
```
*(Cela va créer un fichier `.yaml` dans le dossier où tu te trouves).*

* **Charger un fichier de paramètres au lancement d'un nœud :**
```bash
ros2 run <nom_du_package> <nom_du_executable> --ros-args --params-file <chemin_vers_fichier.yaml>
```
*(Cette commande indique à ROS 2 : "Lance ce nœud, mais utilise ce fichier pour régler ses paramètres par défaut").*