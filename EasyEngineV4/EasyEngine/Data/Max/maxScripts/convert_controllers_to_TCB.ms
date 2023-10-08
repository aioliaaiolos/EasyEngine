format "Début du script"
select $*mixamo*

for o in selection do 
(
   o.position.controller = TCB_Position()
)

for o in selection do 
(
	o.rotation.controller = TCB_Rotation()
)
   
for o in selection do 
(
	o.scale.controller = TCB_Scale()
)

print "Fin du script"
