import bpy
import sys

argv = sys.argv
argv = argv[argv.index("--") + 1:] # get all args after "--"

stl_in = argv[0]
#stl_in = '/home/austin/Documents/GitHub/PFIAir/uploads/models/bolt.obj'

filename = stl_in.split('/')[-1]

previewFilename = argv[1]

if(filename.split('.')[1] == "stl" or filename.split('.')[1] == "STL"):
	bpy.ops.import_mesh.stl(filepath=stl_in)
	print("stl")
else:
	raise Exception('file type not supported')

bpy.context.scene.render.filepath = previewFilename
bpy.context.scene.render.use_overwrite = True

objects = bpy.context.selected_objects
try:
	bpy.context.scene.objects.active = objects[0]
except IndexError:
	print(bpy.context.selected_objects)
	raise Exception('file import failed')
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
