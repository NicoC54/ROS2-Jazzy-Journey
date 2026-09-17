# Fiche de Cours Complète : Les Nœuds et les Topics dans ROS 2 (Jazzy)

## Partie 1 : Le Cours Détailé

### 1. Introduction : La philosophie de ROS 2
Imagine que tu dois construire un robot complet. Plutôt que d'écrire un seul programme gigantesque (qui serait difficile à débugger et à mettre à jour), ROS 2 t'encourage à diviser le travail en de multiples petits sous-programmes. C'est ce qu'on appelle une architecture modulaire.

### 2. Qu'est-ce qu'un Nœud (Node) ?
**Définition :** Un Nœud est un programme exécutable individuel, responsable d'une tâche unique et précise.
*   **L'idée de base :** Dans un robot, tu auras un nœud pour lire les données d'un laser, un autre pour calculer la position, et encore un autre pour faire tourner les moteurs. 
*   **Pourquoi faire cela ?** Si le nœud de la caméra plante, le nœud des moteurs continue de fonctionner. Cela rend le système extrêmement robuste. 

### 3. Comment les Nœuds se parlent-ils ? Les Topics (Sujets)
Puisque les nœuds sont isolés, ils doivent communiquer pour que le robot fonctionne comme un tout. La méthode de communication la plus courante pour les flux de données continus s'appelle le **Topic**.

**Définition :** Un Topic est un canal de communication unidirectionnel continu (comme une fréquence radio) sur lequel les nœuds s'échangent des données.

Pour qu'un Topic fonctionne, on utilise un modèle appelé **Publisher / Subscriber** (Éditeur / Abonné) :
*   **Le Publisher (Éditeur) :** C'est le nœud qui génère la donnée et l'envoie sur le Topic. *Exemple : Le nœud du capteur de température publie (envoie) la température en continu.*
*   **Le Subscriber (Abonné) :** C'est le nœud qui a besoin de l'information. Il "s'abonne" au Topic pour écouter les données qui y circulent. *Exemple : Le nœud de l'écran d'affichage s'abonne au topic de température pour l'afficher.*

**Connexion des idées :** Un Topic n'appartient à personne. Un nœud peut publier sur un Topic sans savoir si quelqu'un écoute. De même, plusieurs nœuds peuvent s'abonner au même Topic simultanément. C'est un flux continu, idéal pour les capteurs (caméras, radars, odométrie).

### 4. Le langage commun : Les Messages (Interfaces)
Pour qu'un Subscriber comprenne ce qu'un Publisher envoie, ils doivent parler le même langage.
**Définition :** Un message est une structure de données stricte qui définit le type d'information circulant sur un Topic.
*   Si un nœud publie sur un Topic appelé `/vitesse_robot`, le message associé dictera s'il s'agit d'un nombre entier (int), d'un texte (string), ou d'une structure complexe (ex: `geometry_msgs/msg/Twist` qui contient des coordonnées X, Y, Z).

---

## Partie 2 : Les Commandes Utiles (CLI)

Voici l'arsenal des commandes pour inspecter et manipuler les Nœuds et les Topics depuis le terminal.

### 1. Gestion des Nœuds (Nodes)
* **Lister tous les nœuds en cours d'exécution :**
```bash
ros2 node list
```
*(Note : Il faut qu'un programme tourne pour voir des nœuds).*

* **Afficher les informations détaillées d'un nœud spécifique :**
```bash
ros2 node info /<nom_du_noeud>
```
*(Permet de voir tous les Publishers, Subscribers, Services et Actions liés à ce nœud).*

### 2. Exploration et Analyse des Topics
* **Lister tous les topics actifs :**
```bash
ros2 topic list
```

* **Lister les topics avec leur type de message associé :**
```bash
ros2 topic list -t
```

* **Afficher les métadonnées d'un topic :**
```bash
ros2 topic info /<nom_du_topic>
```
*(Affiche le nombre de Publishers, de Subscribers, et le type de message).*

* **Voir la structure interne d'un type de message :**
```bash
ros2 interface show <type_de_message>
```
*(Exemple : `ros2 interface show geometry_msgs/msg/Twist`)*

### 3. Interaction en Direct avec les Topics
* **Écouter et afficher en direct les données d'un topic :**
```bash
ros2 topic echo /<nom_du_topic>
```

* **Vérifier la fréquence de publication (en Hertz) d'un topic :**
```bash
ros2 topic hz /<nom_du_topic>
```

* **Publier un message manuellement UNE SEULE FOIS depuis le terminal :**
```bash
ros2 topic pub --once /<nom_du_topic> <type_de_message> "{argument: valeur}"
```

* **Publier un message manuellement EN CONTINU (à une fréquence définie) :**
```bash
ros2 topic pub --rate <frequence_en_Hz> /<nom_du_topic> <type_de_message> "{argument: valeur}"
```

### 4. Visualisation Graphique
* **Générer une carte visuelle de l'architecture :**
```bash
rqt_graph
```
*(Ouvre une fenêtre graphique montrant les nœuds sous forme de bulles et les topics sous forme de flèches qui les relient).*