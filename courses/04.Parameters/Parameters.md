# Fiche de Révision Ingénieur : Les Paramètres dans ROS 2 (Jazzy)

## 1. Concepts Fondamentaux

**L'analogie du jeu vidéo :** Comme dans le menu "Options" d'un jeu (volume, sensibilité), les paramètres ROS 2 permettent de modifier le comportement d'un nœud sans avoir à recompiler le code ou redémarrer le robot.

* **Appartenance locale (Scope) :** Contrairement à ROS 1 (où le *Parameter Server* était global), les paramètres ROS 2 sont décentralisés. Chaque nœud possède et gère son propre dictionnaire de paramètres.
* **Typage strict :** En C++, le type d'un paramètre est figé. Les types autorisés sont : `bool`, `int64`, `double`, `string`, `byte_array`, `bool_array`, `integer_array`, `double_array`, `string_array`.
* **Sous le capot (L'architecture) :** Techniquement, il n'y a pas de "magie". Lorsqu'un nœud est créé, ROS 2 génère automatiquement des **Services** invisibles (`~/set_parameters`, `~/get_parameters`, `~/describe_parameters`). Interagir avec un paramètre revient en réalité à faire un appel de service standard.

## 2. Le Cycle de Vie dans le Code (Règles d'Ingénierie)

Pour qu'un paramètre existe et soit modifiable, le développeur doit respecter 3 étapes clés :

1. **L'obligation de déclaration :** En ROS 2, un paramètre **doit** être déclaré dans le code avant toute interaction. Si un Launch file ou le terminal tente de modifier un paramètre non déclaré, le nœud crashera avec l'erreur `ParameterNotDeclaredException`.
2. **La lecture initiale :** Au démarrage, le code lit la valeur finale (qui peut avoir été écrasée par le Launch file) pour l'injecter dans la logique de l'algorithme (ex: initialiser la variable membre `vitesse_max_`).
3. **La reconfiguration dynamique (Le Callback) :** Modifier un paramètre via le terminal ne change pas automatiquement le comportement du robot. Le développeur doit implémenter un **Parameter Callback**. Cette fonction intercepte la demande, vérifie sa validité (elle peut la rejeter en renvoyant `false`), et met à jour les variables C++ internes.

## 3. Déploiement et Opérations (CLI & Launch Files)

### Intégration en Production (Launch Files)

En entreprise, on n'utilise pas `ros2 run`. On injecte les paramètres via des Launch files Python, soit individuellement, soit via un fichier complet `config.yaml`.

```python
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='mon_pkg',
            executable='mon_exec',
            name='mon_noeud',
            parameters=[
                {'vitesse_limite': 3.5},             # 1. Injection d'un paramètre unique
                'chemin/vers/configuration.yaml'     # 2. Injection massive via fichier YAML
            ]
        )
    ])

```

### Commandes Utiles (CLI) pour le Debug

* **Lister les paramètres :** `ros2 param list`
* **Connaître le type :** `ros2 param describe /nom_noeud nom_parametre`
* **Lire la valeur :** `ros2 param get /nom_noeud nom_parametre`
* **Modifier à chaud :** `ros2 param set /nom_noeud nom_parametre nouvelle_valeur`
* **Sauvegarder la config :** `ros2 param dump /nom_noeud` (génère un `.yaml` de l'état actuel).

---

## 4. Implémentation C++ de Référence (`rclcpp`)

Voici le code standard attendu d'un ingénieur pour gérer proprement un paramètre modifiable à la volée.

```cpp
#include "rclcpp/rclcpp.hpp"
#include "rcl_interfaces/msg/set_parameters_result.hpp"

class MonNoeud : public rclcpp::Node {
public:
    MonNoeud() : Node("mon_noeud_parametres") {
        
        // 1. DÉCLARATION OBLIGATOIRE (avec valeur par défaut)
        this->declare_parameter<double>("vitesse_limite", 1.0);

        // 2. LECTURE INITIALE (pour récupérer la valeur écrasée par le launch file si elle existe)
        this->get_parameter("vitesse_limite", vitesse_actuelle_);
        RCLCPP_INFO(this->get_logger(), "Démarrage avec vitesse : %f", vitesse_actuelle_);

        // 3. ENREGISTREMENT DU CALLBACK DE RECONFIGURATION DYNAMIQUE
        // Crucial : Il faut stocker le retour dans callback_handle_ pour qu'il ne soit pas détruit
        callback_handle_ = this->add_on_set_parameters_callback(
            std::bind(&MonNoeud::paramModificationCallback, this, std::placeholders::_1)
        );
    }

private:
    double vitesse_actuelle_;
    std::shared_ptr<rclcpp::node_interfaces::OnSetParametersCallbackHandle> callback_handle_;

    // 4. LA FONCTION DE RAPPEL (CALLBACK)
    rcl_interfaces::msg::SetParametersResult paramModificationCallback(
        const std::vector<rclcpp::Parameter> &parameters) 
    {
        rcl_interfaces::msg::SetParametersResult result;
        result.successful = true;
        result.reason = "Succès";

        for (const auto &param : parameters) {
            if (param.get_name() == "vitesse_limite") {
                // Vérification du type imposé par le C++
                if (param.get_type() == rclcpp::ParameterType::PARAMETER_DOUBLE) {
                    double nouvelle_vitesse = param.as_double();
                    
                    // Logique métier : on peut rejeter une valeur dangereuse
                    if (nouvelle_vitesse < 0.0) {
                        result.successful = false;
                        result.reason = "La vitesse ne peut pas être négative !";
                    } else {
                        vitesse_actuelle_ = nouvelle_vitesse;
                        RCLCPP_INFO(this->get_logger(), "Vitesse mise à jour à : %f", vitesse_actuelle_);
                    }
                }
            }
        }
        return result;
    }
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MonNoeud>());
    rclcpp::shutdown();
    return 0;
}

```