En robotique mathématique, le temps n'est pas un simple outil d'horodatage pour les logs. C'est une variable fondamentale ($t$ et $\Delta t$) utilisée en permanence pour calculer des dérivées, estimer des vitesses (PID), ou fusionner des données spatiales (TF2). Si ton robot se trompe sur la valeur du temps, toutes ses mathématiques s'effondrent.

Pour comprendre comment ROS 2 gère ce défi, il faut d'abord définir et séparer trois horloges différentes.

**1. L'horloge Système (Wall Time)**
C'est l'horloge matérielle de ton ordinateur. Elle donne l'heure réelle (ex: mardi 14h30). Son problème ? Elle peut être modifiée par l'utilisateur ou synchronisée via internet en plein vol, ce qui ferait faire un "bond" temporel illogique à tes algorithmes.

**2. L'horloge Monotone (Steady Time)**
C'est un chronomètre matériel interne à l'ordinateur qui démarre quand tu l'allumes. Il ne recule **jamais** et avance toujours à la vitesse exacte de 1 seconde physique par seconde. C'est l'outil parfait pour calculer la durée exacte d'exécution d'un bout de code en C++, mais il ne connaît pas l'heure qu'il est.

**3. L'horloge ROS (ROS Time ou Sim Time)**
C'est l'horloge virtuelle de l'écosystème ROS 2. C'est le concept central à maîtriser.

Voici pourquoi le *ROS Time* existe :
Imagine que tu as enregistré un flux de données (un *rosbag*) d'un robot réel qui a roulé pendant 1 heure. Quand tu rentres au bureau, tu veux rejouer cette donnée dans ton algorithme pour le tester, mais tu veux le rejouer en avance rapide (x2).
Si ton algorithme de navigation regarde l'horloge matérielle de ton ordinateur (*Wall Time*), il va voir que 30 minutes réelles se sont écoulées, mais il aura reçu 1 heure de données capteurs. Tes vitesses calculées par ton PID seront fausses de 50%. Le robot simulé va s'écraser.

La solution de l'architecture ROS 2 est de forcer tous les nœuds à ne jamais regarder la montre de l'ordinateur, mais à utiliser une abstraction : `rclcpp::Clock`.

**La connexion des concepts : Le paramètre `use_sim_time` et le Topic `/clock**`
Par défaut, l'horloge ROS copie l'horloge système (elle avance normalement).
Mais ROS 2 possède un paramètre natif appelé `use_sim_time`. Si tu mets ce paramètre à `true` lorsque tu lances ton nœud, l'abstraction s'active :

1. Ton nœud se déconnecte totalement de la carte mère de ton ordinateur.
2. Il s'abonne automatiquement, en arrière-plan, à un topic ROS spécial qui s'appelle `/clock`.
3. Dès lors, le temps n'avance pour ton nœud que lorsqu'il reçoit un message sur ce topic.

Si un simulateur (comme Gazebo) ou un *rosbag* publie sur le topic `/clock` à une vitesse accélérée (x2), ralentie (x0.5), ou même fait une pause, ton nœud lira ce temps virtuel. Pour tes algorithmes mathématiques, le temps s'écoulera parfaitement en synchronisation avec les données reçues, peu importe la vitesse du monde réel.

# ⏱️ Le Système Temporel ROS 2 (Time System)

> **Objectif :** Maîtriser l'abstraction temporelle de ROS 2 pour garantir la stabilité des algorithmes mathématiques (PID, Odométrie, TF2) lors des transitions entre un robot physique et une simulation.

## 1. Les trois types d'horloges (Clock Types)

ROS 2 définit trois horloges via l'énumération `rcl_clock_type_t` :
- **`RCL_SYSTEM_TIME` (Wall Time) :** L'heure de l'ordinateur (synchronisable via NTP). Peut sauter en avant ou en arrière. À éviter pour les calculs de durée.
- **`RCL_STEADY_TIME` :** Chronomètre strictement monotone. Ne recule jamais. Idéal pour mesurer le temps d'exécution (benchmarking) d'une fonction C++.
- **`RCL_ROS_TIME` (Par défaut) :** L'horloge dynamique. Copie le `SYSTEM_TIME` par défaut, mais peut être surchargée par une source de temps externe (simulation) si le paramètre `use_sim_time` est activé.

## 2. L'Objet `rclcpp::Time` et `rclcpp::Duration`

Dans ROS 2, le temps n'est pas un simple `double` ou un `int`. C'est un objet précis structuré en deux parties pour éviter les erreurs d'arrondi :
- Les **secondes** (entier).
- Les **nanosecondes** (entier).

- **`Time` :** Représente un moment précis dans l'univers (ex: l'instant où un scan LIDAR a été capturé).
- **`Duration` :** Représente un intervalle mathématique ($\Delta t$) entre deux objets `Time`.

## 3. Le mécanisme `use_sim_time`

C'est la clé de voûte de la simulation spatio-temporelle.
- **`use_sim_time = false` (Robot Réel) :** L'appel à `this->get_clock()->now()` lit le processeur physique.
- **`use_sim_time = true` (Simulation/Bag) :** L'appel à `this->get_clock()->now()` lit la dernière valeur reçue sur le topic `/clock`. Si Gazebo est en pause, `now()` retournera la même valeur à l'infini, mettant le nœud en "stase" temporelle.

---

## 4. Implémentation C++

### A. Obtenir le temps courant
C'est la méthode standard pour dater un message avant de le publier.

```cpp
#include "rclcpp/rclcpp.hpp"

class TimeNode : public rclcpp::Node {
public:
    TimeNode() : Node("time_node") {
        // Récupère l'horloge ROS du nœud et demande l'instant 'T' actuel
        rclcpp::Time current_time = this->get_clock()->now();
        
        RCLCPP_INFO(this->get_logger(), "Secondes : %f", current_time.seconds());
    }
};

```

### B. Calculer une durée (Delta T pour un PID)

Pour calculer un dérivé (ex: vitesse = distance / temps), il faut soustraire deux objets `Time`.

```cpp
rclcpp::Time t_start = this->get_clock()->now();

// ... (Exécution d'un long calcul ou attente du prochain cycle) ...

rclcpp::Time t_end = this->get_clock()->now();

// La soustraction génère un objet Duration
rclcpp::Duration delta_t = t_end - t_start;

RCLCPP_INFO(this->get_logger(), "Le calcul a pris %f secondes", delta_t.seconds());

```

### C. Forcer un type d'horloge spécifique

Parfois, on veut forcer l'utilisation de l'horloge matérielle, indépendamment de la simulation (ex: pour mesurer les performances CPU d'un algorithme).

```cpp
// Instancie une horloge indépendante qui refuse d'écouter le ROS Time
rclcpp::Clock steady_clock(RCL_STEADY_TIME);

rclcpp::Time start = steady_clock.now();
// ... code ...
rclcpp::Duration duration = steady_clock.now() - start;

```

```

