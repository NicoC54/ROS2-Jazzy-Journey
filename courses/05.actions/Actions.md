# Fiche de Cours Complète : Les Actions dans ROS 2 (Jazzy)

## Partie 1 : Le Cours Détaillé

### 1. Introduction : Les limites des Services
Nous avons vu que les **Services** sont parfaits pour des requêtes rapides (ex: "Allume la lumière", "Prends une photo"). Le Client demande, attend quelques millisecondes, et reçoit la réponse. 
Mais que se passe-t-il si la tâche demande **beaucoup de temps** ? Par exemple : "Fais avancer le robot jusqu'à la cuisine à 50 mètres". 
Si on utilise un Service, le nœud Client va rester "bloqué" (figé) à attendre la réponse pendant les 2 minutes de trajet. De plus, il n'aura aucune idée de l'avancement du robot, et il lui sera impossible d'annuler l'ordre. C'est pour résoudre ce problème précis qu'ont été créées les **Actions**.

### 2. Qu'est-ce qu'une Action ?
**Définition :** Une Action est un mode de communication asynchrone complexe, conçu pour exécuter des tâches longues. Elle permet au Client d'envoyer une requête, de recevoir des mises à jour en continu sur l'état d'avancement, de recevoir un résultat final, et surtout, d'annuler la tâche en cours de route si nécessaire.

Comme les Services, les Actions fonctionnent avec un modèle **Client / Serveur** :
*   **Action Client :** Celui qui donne l'ordre (envoie le but).
*   **Action Serveur :** Celui qui exécute la tâche longue.

*À noter (Pour aller plus loin) : En réalité, sous le capot de ROS 2, une Action n'est pas une nouvelle technologie magique. Elle est construite en combinant intelligemment plusieurs Topics et Services en même temps !*

### 3. Les trois composantes d'une Action
Pour qu'une action fonctionne, la communication est divisée en trois éléments distincts :

1.  **Le But (Goal) :** C'est la requête initiale envoyée par le Client au Serveur. *Exemple : "Va aux coordonnées X=10, Y=5".*
2.  **Le Retour d'état (Feedback) :** C'est un flux de données continu envoyé par le Serveur au Client PENDANT l'exécution de la tâche. *Exemple : "Je suis à X=2", puis "Je suis à X=4", etc.*
3.  **Le Résultat (Result) :** C'est le message final envoyé par le Serveur une fois la tâche totalement terminée (ou échouée). *Exemple : "Trajet terminé avec succès, je suis arrivé".*

### 4. Le langage commun : L'Interface (.action)
Tout comme les Topics utilisent des messages (`.msg`) et les Services utilisent des (`.srv`), les Actions utilisent un fichier d'interface spécifique : le fichier **`.action`**.
**Structure :** Ce fichier contient la définition des 3 composantes, séparées par des triples tirets (`---`).
*   **Partie 1 (en haut) :** La structure du Goal (Requête).
*   **Partie 2 (au milieu) :** La structure du Result (Résultat final).
*   **Partie 3 (en bas) :** La structure du Feedback (Mise à jour en temps réel).

---

## Partie 2 : Les Commandes Utiles (CLI)

Voici comment inspecter et interagir avec les Actions depuis ton terminal.

### 1. Explorer le réseau d'Actions
* **Lister toutes les Actions actuellement disponibles sur le réseau :**
```bash
ros2 action list
```

* **Afficher les nœuds Clients et Serveurs impliqués dans une action spécifique :**
```bash
ros2 action info /<nom_de_l_action>
```

### 2. Comprendre l'Interface d'une Action
* **Voir la structure interne d'une interface d'Action (le contenu du fichier .action) :**
```bash
ros2 interface show <type_de_l_action>
```
*(Indispensable pour connaître les champs exacts du Goal, du Result et du Feedback).*

### 3. Lancer une Action manuellement (Débogage)
* **Envoyer un But (Goal) depuis le terminal :**
```bash
ros2 action send_goal /<nom_de_l_action> <type_de_l_action> "<arguments_yaml>"
```
*(Attention : Cette commande envoie le but et attend le résultat final, mais elle n'affiche pas le feedback).*

* **Envoyer un But ET afficher le Retour d'état (Feedback) en temps réel :**
```bash
ros2 action send_goal --feedback /<nom_de_l_action> <type_de_l_action> "<arguments_yaml>"
```
*(L'option `--feedback` est cruciale pour voir les données défiler pendant que l'action s'exécute).*