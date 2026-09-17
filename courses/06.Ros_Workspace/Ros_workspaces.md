# Fiche de Cours Complète : Espace de Travail (Workspace) et Colcon dans ROS 2 (Jazzy)

## Partie 1 : Le Cours Détaillé

### 1. Introduction : L'analogie de l'Atelier
Imagine que tu es un artisan (un développeur) et que tu veux fabriquer un robot. 
*   Tu ne vas pas travailler au milieu de ton salon (le reste de ton ordinateur). Tu vas te créer un **Atelier** dédié : c'est le **Workspace**.
*   Dans cet atelier, tu as besoin de tiroirs ou de boîtes pour ranger tes différents projets sans tout mélanger (une boîte pour la caméra, une pour les moteurs) : ce sont les **Packages**.
*   Enfin, tu as besoin d'une machine-outil automatisée qui prend tes plans et construit les pièces finales : cette machine s'appelle **Colcon**.

### 2. Qu'est-ce qu'un Workspace (Espace de Travail) ?
**Définition :** Un Workspace est simplement un dossier sur ton ordinateur dans lequel tu vas créer, modifier et compiler (construire) ton code ROS 2. 

Un Workspace standard est toujours organisé en 4 sous-dossiers principaux (dont 3 sont générés automatiquement) :
1.  **`src/` (Source)** : C'est LE dossier le plus important pour toi. C'est ici que tu écris ton code et que tu places tes Packages. C'est la matière première.
2.  **`build/`** : Dossier généré automatiquement. Il contient les fichiers temporaires créés pendant que la machine assemble ton code. Tu n'as jamais besoin d'y toucher.
3.  **`install/`** : Dossier généré automatiquement. C'est le produit fini. Il contient tes programmes exécutables (les Nœuds) prêts à être lancés.
4.  **`log/`** : Dossier généré automatiquement contenant les journaux (logs) de compilation. Utile uniquement si la compilation échoue pour comprendre l'erreur.

### 3. Qu'est-ce qu'un Package (Paquet) ?
**Définition :** Un Package est l'unité d'organisation de base du code dans ROS 2. C'est un dossier (situé dans `src/`) qui regroupe tout ce qui est nécessaire pour accomplir une tâche spécifique : le code (C++ ou Python), les fichiers de configuration, et un fichier décrivant comment le construire (`package.xml`).
*Règle d'or : Dans ROS 2, tout ton code DOIT être à l'intérieur d'un Package. Tu ne peux pas avoir de code qui "flotte" librement dans le Workspace.*

### 4. Qu'est-ce que Colcon ?
**Définition :** Colcon est l'outil de compilation officiel de ROS 2. C'est le logiciel qui va analyser tout ce qui se trouve dans ton dossier `src/`, traduire ton code en programmes exécutables, et ranger proprement le résultat dans le dossier `install/`. 

### 5. Le concept vital : Le "Sourcing" (Overlay / Underlay)
C'est souvent l'étape la plus compliquée à comprendre au début.
*   Quand tu ouvres un terminal sous Linux, il ne connaît pas l'existence de ROS 2, ni de ton Workspace.
*   **"Sourcer"**, c'est exécuter un petit script (`setup.bash`) qui va dire à ton terminal : "Voici où se trouvent les outils ROS 2 et voici où se trouvent mes propres programmes".
*   **L'Underlay (La base) :** C'est l'installation principale de ROS 2 sur ton ordinateur (ex: `/opt/ros/jazzy`). Tu dois toujours la sourcer en premier.
*   **L'Overlay (Ta surcouche) :** C'est TON workspace. Une fois compilé, tu dois sourcer le dossier `install/` pour que le terminal trouve tes nouveaux nœuds. L'Overlay vient s'ajouter par-dessus l'Underlay.

---

## Partie 2 : Les Commandes Utiles (CLI)

Voici l'enchaînement classique pour gérer ton espace de travail au quotidien.

### 1. Création de l'Espace de Travail
* **Créer le dossier principal et le sous-dossier src (ex: `ros2_ws`) :**
```bash
mkdir -p ~/ros2_ws/src
```

* **Se déplacer dans le workspace (Toujours se placer à la racine avant de compiler !) :**
```bash
cd ~/ros2_ws
```

### 2. Création d'un Package (à faire dans le dossier `src/`)
* **Créer un package en Python :**
```bash
cd ~/ros2_ws/src
ros2 pkg create --build-type ament_python <nom_du_package>
```
* **Créer un package en C++ :**
```bash
cd ~/ros2_ws/src
ros2 pkg create --build-type ament_cmake <nom_du_package>
```

### 3. Compilation avec Colcon (à faire à la RACINE du workspace, ex: `~/ros2_ws`)
* **Compiler tout le workspace :**
```bash
colcon build
```

* **Compiler un seul package spécifique (pour gagner du temps) :**
```bash
colcon build --packages-select <nom_du_package>
```

* **Compiler avec l'option Symlink (Astuce pour Python) :**
```bash
colcon build --symlink-install
```
*(Cette option permet de modifier un fichier Python et de tester directement sans avoir à recompiler à chaque fois).*

### 4. Le Sourcing (Indispensable avant de lancer `ros2 run`)
* **Sourcer l'installation de base de ROS 2 (Underlay) :**
```bash
source /opt/ros/jazzy/setup.bash
```

* **Sourcer ton propre espace de travail (Overlay) :**
```bash
source install/setup.bash
```
*(À faire à la racine de ton workspace, après chaque nouvelle compilation `colcon build`).*