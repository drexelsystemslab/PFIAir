import bpy
import sys

argv = sys.argv

threeds_in = argv[5]
#stl_in = '/home/austin/Documents/GitHub/PFIAir/uploads/models/bolt.obj'

#filename = threeds_in.split('/')[-1]

targetFilename = argv[6]

# deselect all
bpy.ops.object.select_all(action='DESELECT')

objects_on_1st_layer = [ob for ob in bpy.context.scene.objects if ob.layers[0]]

for obj in objects_on_1st_layer:
    obj.select = True
    bpy.ops.object.delete()


bpy.ops.import_scene.autodesk_3ds(filepath=threeds_in, axis_forward='Y', axis_up='Z', filter_glob="*.3ds", constrain_size=10.0, use_image_search=True, use_apply_transform=True)
print("IMPORTED")
bpy.ops.export_mesh.stl(filepath=targetFilename, check_existing=True, axis_forward='Y', axis_up='Z', filter_glob="*.stl", use_selection=False, global_scale=1.0, use_scene_unit=False, ascii=False, use_mesh_modifiers=True, batch_mode='OFF')
print("EXPORTED")