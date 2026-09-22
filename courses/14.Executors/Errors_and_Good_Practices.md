
```markdown
# 🚨 C++ & ROS 2 : Common Errors and Good Practices

> **Objectif :** Ce document recense les erreurs classiques lors de l'implémentation de nœuds ROS 2 en C++ et définit les bonnes pratiques architecturales pour garantir une compilation sans erreur et un code *production-ready*.

---

## 1. Erreurs de structure C++ (Les fondamentaux)

### ❌ Affectation hors d'une portée d'exécution
- **L'erreur :** Déclarer et affecter une valeur à une variable (ex: configuration de struct) directement dans le corps de la classe, en dehors de toute méthode.
- **La correction :** Déplacer toute logique d'affectation à l'intérieur du constructeur ou d'une méthode dédiée.
- **🧠 Le réflexe (Best Practice) :** *Le corps d'une classe (`class MyNode { ... }`) ne sert qu'à **déclarer** l'existence des variables. Toute action dynamique doit se trouver dans une méthode d'exécution (comme le constructeur).*

### ❌ Erreur de nommage et variables fantômes
- **L'erreur :** Déclarer `groupe_exclusif` mais utiliser `mutual_exclusive` plus loin dans le code.
- **La correction :** Respecter strictement la casse et l'orthographe de la déclaration.
- **🧠 Le réflexe (Best Practice) :** *Ne jamais taper un nom de variable en entier. Taper les 3 premières lettres et utiliser l'autocomplétion (`Tab`). Si l'IDE ne propose rien, c'est que la variable est hors de portée (scope) ou mal orthographiée.*

---

## 2. Erreurs spécifiques à l'API ROS 2 (Jazzy)

### ❌ L'oubli des `std::placeholders` dans `std::bind`
- **L'erreur :** Lier une fonction de callback qui prend des arguments sans spécifier les "trous" à remplir par ROS 2.
  ```cpp
  // Faux : le callback attend un message, mais le bind ne lui laisse pas de place.
  std::bind(&MyNode::Callback, this)

```

* **La correction :** Ajouter `std::placeholders::_1` (ou `_2`, etc. selon le nombre d'arguments).
```cpp
// Vrai
std::bind(&MyNode::Callback, this, std::placeholders::_1)

```


* **🧠 Le réflexe (Best Practice) :** *La règle du "Bind" : Je compte les arguments de mon Callback. S'il prend 1 paramètre (ex: un Subscriber), je DOIS mettre `_1`. S'il n'en prend aucun (ex: un Timer), je ne mets rien.*

### ❌ Omission de la QoS (Quality of Service)

* **L'erreur :** Créer un Subscriber ou un Publisher sans spécifier la taille de la file d'attente (History depth).
* **La correction :** Ajouter systématiquement la valeur entière (ex: `10`) après le nom du topic.
```cpp
this->create_subscription<std_msgs::msg::String>("topic_name", 10, callback);

```


* **🧠 Le réflexe (Best Practice) :** *En ROS 2, un topic n'existe pas sans QoS. Penser systématiquement au trio : `<Type_Message>("Nom_du_canal", Taille_File_Attente, Action)`.*

### ❌ Ignorer les alias de Pointeurs Intelligents (Smart Pointers)

* **L'erreur :** Utiliser la syntaxe standard lourde du C++ pour les objets de l'API ROS.
```cpp
std::shared_ptr<rclcpp::TimerBase> timer_;

```


* **La correction :** Utiliser les alias natifs de ROS 2.
```cpp
rclcpp::TimerBase::SharedPtr timer_;

```


* **🧠 Le réflexe (Best Practice) :** *Toujours privilégier `::SharedPtr` fournit par les classes de l'écosystème ROS 2 pour garder un code clair et idiomatique.*

### ❌ Typographie des classes ROS 2

* **L'erreur :** Écrire `MultiThreadExecutors` ou `MutualExclusive`.
* **La correction :** `MultiThreadedExecutor`, `MutuallyExclusive`.
* **🧠 Le réflexe (Best Practice) :** *L'API ROS 2 utilise des adjectifs passés/adverbes pour ses états. Faire confiance au linter C++ de l'IDE.*

---

## ✅ Checklist de Pré-Compilation

Avant de lancer la commande `colcon build`, appliquez ce filtre mental :

* [ ] **Scope de l'action :** Mon code d'affectation est-il bien contenu dans mon constructeur ?
* [ ] **Check Bind :** Mes Callbacks de topics ont-ils bien leur `std::placeholders::_1` ?
* [ ] **Check QoS :** Ai-je bien spécifié un entier (ex: `10`) ou un profil QoS pour chaque Topic créé ?
* [ ] **Check Pointeurs :** Mes attributs `private:` utilisent-ils bien la syntaxe `::SharedPtr` ?
* [ ] **Check Terminologie :** Mes Executors et Options sont-ils bien au singulier ?
* [ ] **Syntaxe C++ :** Ai-je bien mis un `;` à la fin de chaque instruction (particulièrement dans le `main`) ?

```