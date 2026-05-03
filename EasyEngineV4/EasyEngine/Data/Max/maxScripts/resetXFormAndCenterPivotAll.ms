-- Étape 1 : Sélectionner tous les objets géométriques
select geometry

-- Étape 2 : Centrer le pivot de chaque objet
for obj in selection do (
    obj.pivot = obj.center
)

-- Étape 3 : Appliquer le Reset XForm
for obj in selection do (
    resetXForm obj
)

-- Étape 4 : Collapser la pile (Collapse All)
for obj in selection do (
    collapseStack obj
)
