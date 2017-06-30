import bpy
import sys

argv = sys.argv
argv = argv[argv.index("--") + 1:] # get all args after "--"

stl_in = argv[0]
#stl_in = '/home/austin/Documents/GitHub/PFIAir/uploads/models/bolt.obj'

filename = stl_in.split('/')[-1]

previewFilename = argv[1]

if(filename.split('.')[1] == "stl"):
	bpy.ops.import_mesh.stl(filepath=stl_in)
	print("stl")
elif(filename.split('.')[1] == "obj"):
	bpy.ops.import_scene.obj(filepath=stl_in)
	print("obj")

bpy.context.scene.render.filepath = previewFilename
bpy.context.scene.render.use_overwrite = True

objects = bpy.context.selected_objects
bpy.context.scene.objects.active = objects[0]
x,y,z = bpy.context.active_object.dimensions

maxDimension = max(x,y,z)

x1 = (2*x)/maxDimension
y1 = (2*y)/maxDimension
z1 = (2*z)/maxDimension

print(x)
print(y)
print(z)
print(x1)
print(y1)
print(z1)

bpy.context.active_object.dimensions = x1,y1,z1

bpy.context.scene.render.resolution_x = 800 #perhaps set resolution in code
bpy.context.scene.render.resolution_y = 800
bpy.ops.render.render(write_still=True )
