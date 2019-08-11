import trimesh
import sys

def split(path, no_extension):
    mesh = trimesh.load_mesh(path + no_extension + ".stl")
    print("MESH LOADED")
    split = mesh.split()
    print("MESH SPLIT")
    path = "/Users/jeffwinchell/Desktop/NSF_Research/PFIAir/PFI_Scripts/Split_STL_models/"
    for i, banana in enumerate(split):
      with open(no_extension,"wb+") as file:
        banana.export(path + no_extension + "_"+ str(i) + ".stl","stl")
        print(banana)
        print("EXPORTED")