#This makefile for Windows was originally made by Dominic Calkosz and made available in the class Discord.
#I did not write this makefile originally, I only edited it for scenes besides hexapod.

all : \
    ../dist/platformer.pnct \
    ../dist/platformer.scene \

../dist/platformer.scene : platformer.blend export-scene.py
    "E:\Program Files\Blender Foundation\Blender 2.90\blender.exe" --background --python export-scene.py -- "platformer.blend:Main" "../dist/platformer.scene"

../dist/platformer.pnct : platformer.blend export-meshes.py
    "E:\Program Files\Blender Foundation\Blender 2.90\blender.exe" --background --python export-meshes.py -- "platformer.blend:Main" "../dist/platformer.pnct" 