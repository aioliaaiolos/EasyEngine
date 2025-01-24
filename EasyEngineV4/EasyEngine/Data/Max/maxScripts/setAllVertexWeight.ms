skinMod = $.modifiers[#Skin]
numVerts = skinOps.GetNumberVertices skinMod

-- ID du bone par défaut à influencer
defaultBoneID = 6 -- Change ce chiffre selon l'ID de ton bone cible

for i = 1 to numVerts do (
    -- Vérifie si le vertex a des poids assignés
    weightCount = skinOps.GetVertexWeightCount skinMod i

    if weightCount == 0 then (
        -- Assigne un poids de 1.0 au bone spécifié si aucun poids n'est présent
        skinOps.ReplaceVertexWeights skinMod i #(defaultBoneID) #(1.0)
    )
)

format "Tous les vertices ont maintenant un poids minimal.\n"
