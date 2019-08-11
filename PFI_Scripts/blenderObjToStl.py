import bpy
import sys

argv = sys.argv

obj_in = argv[5]
#stl_in = '/home/austin/Documents/GitHub/PFIAir/uploads/models/bolt.obj'

#filename = obj_in.split('/')[-1]

targetFilename = argv[6]

# deselect all
bpy.ops.object.select_all(action='DESELECT')

objects_on_1st_layer = [ob for ob in bpy.context.scene.objects if ob.layers[0]]

for obj in objects_on_1st_layer:
    obj.select = True
    bpy.ops.object.delete()

bpy.ops.import_scene.obj(filepath=obj_in, axis_forward='-Z', axis_up='Y', filter_glob="*.obj;*.mtl", use_edges=True, use_smooth_groups=True, use_split_objects=True, use_split_groups=True, use_groups_as_vgroups=False, use_image_search=True, split_mode='ON', global_clamp_size=0.0)
print("IMPORTED")
bpy.ops.export_mesh.stl(filepath=targetFilename, check_existing=True, axis_forward='Y', axis_up='Z', filter_glob="*.stl", use_selection=False, global_scale=1.0, use_scene_unit=False, ascii=False, use_mesh_modifiers=True, batch_mode='OFF')
print("EXPORTED")