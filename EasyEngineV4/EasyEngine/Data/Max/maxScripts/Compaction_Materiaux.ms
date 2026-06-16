(
    local makeMaterialUnique = true -- laisse true pour ne pas modifier d'autres objets qui partageraient ce matériau
    local obj = selection[1]

    fn getFirstSlotFromMatID mat id =
    (
        local result = 0

        -- priorité à un slot qui contient vraiment un sous-matériau
        for i = 1 to mat.materialIDList.count do
        (
            if result == 0 and mat.materialIDList[i] == id and mat.materialList[i] != undefined do
            (
                result = i
            )
        )

        -- fallback si le slot est vide
        for i = 1 to mat.materialIDList.count do
        (
            if result == 0 and mat.materialIDList[i] == id do
            (
                result = i
            )
        )

        result
    )

    if obj == undefined then
    (
        messageBox "Sélectionne d'abord l'objet."
    )
    else if obj.material == undefined then
    (
        messageBox "L'objet n'a pas de matériau assigné."
    )
    else if not isProperty obj.material #materialIDList then
    (
        messageBox "Le matériau assigné n'est pas un Multi/Sub-Object."
    )
    else
    (
        undo "Compact Material IDs" on
        (
            if makeMaterialUnique do obj.material = copy obj.material

            local mat = obj.material
            local faceCount = getNumFaces obj

            -- Récupère les IDs réellement utilisés par les faces
            local usedIDs = #()

            for f = 1 to faceCount do
            (
                local oldID = getFaceMatID obj f

                if findItem usedIDs oldID == 0 do
                (
                    append usedIDs oldID
                )
            )

            sort usedIDs

            -- Sauvegarde les sous-matériaux correspondant aux IDs utilisés
            local savedSubMats = #()
            local savedNames = #()
            local savedEnabled = #()

            for oldID in usedIDs do
            (
                local slot = getFirstSlotFromMatID mat oldID
                local sm = undefined
                local nm = ""
                local en = true

                if slot != 0 do
                (
                    try sm = mat.materialList[slot] catch()
                    try nm = mat.names[slot] catch()
                    try en = mat.mapEnabled[slot] catch()
                )

                if sm != undefined and nm == "" do nm = sm.name

                append savedSubMats sm
                append savedNames nm
                append savedEnabled en
            )

            -- Réduit le Multi/Sub-Object au nombre exact d'IDs utilisés
            mat.numsubs = usedIDs.count

            -- Réécrit les slots en IDs consécutifs : 1,2,3,4...
            for i = 1 to usedIDs.count do
            (
                mat.materialIDList[i] = i
                mat.materialList[i] = savedSubMats[i]

                try mat.names[i] = savedNames[i] catch()
                try mat.mapEnabled[i] = savedEnabled[i] catch()
            )

            -- Réécrit les IDs des faces selon le même mapping
            for f = 1 to faceCount do
            (
                local oldID = getFaceMatID obj f
                local newID = findItem usedIDs oldID

                if newID != 0 do
                (
                    setFaceMatID obj f newID
                )
            )

            update obj

            format "\n--- Compaction terminée pour % ---\n" obj.name
            for i = 1 to usedIDs.count do
            (
                local smName = "<vide>"
                if savedSubMats[i] != undefined do smName = savedSubMats[i].name

                format "Ancien ID % -> nouvel ID % : %\n" usedIDs[i] i smName
            )
        )
    )
)