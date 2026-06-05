rollout expandRollout "Scene Expansion" width:220
(
    group "Facteur de scale"
    (
        spinner spnX "X:" range:[-1000, 1000, 2.0] type:#float
        spinner spnY "Y:" range:[-1000, 1000, 2.0] type:#float
        spinner spnZ "Z:" range:[-1000, 1000, 2.0] type:#float
    )
    
    group "Centre d'expansion"
    (
        spinner spnCX "X:" range:[-100000, 100000, 0.0] type:#float
        spinner spnCY "Y:" range:[-100000, 100000, 0.0] type:#float
        spinner spnCZ "Z:" range:[-100000, 100000, 0.0] type:#float
        button btnUseSelection "Utiliser pivot sélection" width:200
    )
    
    button btnApply "Appliquer l'expansion" width:200 height:30
    
    on btnUseSelection pressed do
    (
        if selection.count > 0 then
        (
            local p = selection[1].position
            spnCX.value = p.x
            spnCY.value = p.y
            spnCZ.value = p.z
        )
        else messageBox "Aucun objet sélectionné"
    )
    
    on btnApply pressed do
    (
        if objects.count == 0 then
        (
            messageBox "Aucun objet dans la scène !"
        )
        else
        (
            local center = [spnCX.value, spnCY.value, spnCZ.value]
            local sx = spnX.value
            local sy = spnY.value
            local sz = spnZ.value
            
            disableSceneRedraw()
            undo "Scene Expansion" on
            (
                for obj in objects do
                (
                    scale obj [sx, sy, sz]
                    local offset = obj.position - center
                    obj.position = center + [offset.x * sx, offset.y * sy, offset.z * sz]
                )
            )
            enableSceneRedraw()
            redrawViews()
            format "Expansion x[%, %, %] appliquée à % objets\n" sx sy sz objects.count
        )
    )
)

createDialog expandRollout