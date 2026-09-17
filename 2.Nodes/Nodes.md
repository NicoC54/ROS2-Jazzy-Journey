# Fiche de Révision ROS 2 : Nœuds et Topics (Vidéo jusqu'à 17:10)

## 1. Les Nœuds (Nodes)

Un nœud est un programme exécutable responsable d'une tâche précise (ex: lire un capteur, envoyer des commandes moteur). Dans l'écosystème ROS, les nœuds communiquent entre eux pour échanger des données.

* **Lister tous les nœuds en cours d'exécution** :
```bash
ros2 node list

```


*(Il faut lancer un programme, comme `ros2 run turtlesim turtlesim_node`, pour qu'un nœud apparaisse dans cette liste).*
* **Afficher les informations détaillées d'un nœud** :
```bash
ros2 node info /<nom_du_noeud>

```


*Cette commande liste tout ce qui est connecté à ce nœud : ses **Subscribers** (abonnés), ses **Publishers** (éditeurs), ses **Services**, et ses **Actions**.*

---

## 2. Les Topics (Sujets)

Les topics sont les canaux de communication continus qui relient les nœuds entre eux.

* Un nœud **Publisher** (éditeur) envoie un flux de données sur un topic.
* Un nœud **Subscriber** (abonné) se connecte à ce topic pour lire ces données.

### Explorer et analyser les topics

* **Lister tous les topics actifs** :
```bash
ros2 topic list

```


* **Lister les topics en affichant également leur type de message** :
```bash
ros2 topic list -t

```


* **Afficher les métadonnées d'un topic** (nombre d'éditeurs, nombre d'abonnés, et type de message exact) :
```bash
ros2 topic info /<nom_du_topic>

```


* **Voir la structure interne d'un type de message** (les variables qu'il contient, ex: `geometry_msgs/msg/Twist`) :
```bash
ros2 interface show <type_de_message>

```



### Lire et écrire sur les topics

* **Écouter et afficher en direct les données qui transitent sur un topic** :
```bash
ros2 topic echo /<nom_du_topic>

```


*(Exemple : `ros2 topic echo /turtle1/pose` permet de voir les coordonnées de la tortue s'actualiser en temps réel pendant qu'elle bouge).*
* **Vérifier la fréquence de publication d'un topic (en Hertz)** :
```bash
ros2 topic hz /<nom_du_topic>

```


*(Affiche le taux moyen, minimum, maximum et l'écart-type de la réception des messages).*
* **Publier un message manuellement UNE SEULE FOIS** :
```bash
ros2 topic pub --once /<nom_du_topic> <type_de_message> "{argument: valeur}"

```


* **Publier un message manuellement EN CONTINU à une fréquence définie** :
```bash
ros2 topic pub --rate <frequence_en_Hz> /<nom_du_topic> <type_de_message> "{argument: valeur}"

```



---

## 3. Visualisation de l'architecture

* **Lancer l'outil graphique de mappage** :
```bash
rqt_graph

```


*Cette interface visuelle génère un schéma représentant tous vos nœuds actifs (bulles ovales) et la manière dont ils sont connectés entre eux via les topics (flèches).*