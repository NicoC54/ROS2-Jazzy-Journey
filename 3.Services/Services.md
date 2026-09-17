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