import bpy
import sys

argv = sys.argv

wrl_in = argv[5]
#stl_in = '/home/austin/Documents/GitHub/PFIAir/uploads/models/bolt.obj'

wrl_in.split('/')[-1]

targetFilename = argv[6]

# deselect all
bpy.ops.object.select_all(action='DESELECT')

objects_on_1st_layer = [ob for ob in bpy.context.scene.objects if ob.layers[0]]

for obj in objects_on_1st_layer:
    obj.select = True
    bpy.ops.object.delete()

bpy.ops.import_scene.x3d(filepath=wrl_in, axis_forward='Z', axis_up='Y', filter_glob="*.x3d;*.wrl")
print("IMPORTED")
bpy.ops.export_mesh.stl(filepath=targetFilename, check_existing=True, axis_forward='Y', axis_up='Z', filter_glob="*.stl", use_selection=False, global_scale=1.0, use_scene_unit=False, ascii=False, use_mesh_modifiers=True, batch_mode='OFF')
print("EXPORTED")