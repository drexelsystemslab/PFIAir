import os
from shutil import copyfile

def get_model_file():
    parent = "/Users/jeffwinchell/Desktop/models/"
    for folder in os.listdir(parent):
        if os.path.isfile("%s%s/model/%s.obj" % (parent,folder, folder)):
            copyfile("%s%s/model/%s.obj" % (parent,folder, folder), "/Users/jeffwinchell/Desktop/NSF_Research/PFIAir/PFI_Scripts/models/%s.obj" % folder)
        elif os.path.isfile("%s%s/model/%s.3ds" % (parent,folder, folder)):
            copyfile("%s%s/model/%s.3ds" % (parent,folder, folder), "/Users/jeffwinchell/Desktop/NSF_Research/PFIAir/PFI_Scripts/models/%s.3ds" % folder)
        elif os.path.isfile("%s%s/model/%s.stl" % (parent,folder, folder)):
            copyfile("%s%s/model/%s.stl" % (parent,folder, folder), "/Users/jeffwinchell/Desktop/NSF_Research/PFIAir/PFI_Scripts/models/%s.stl" % folder)
        elif os.path.isfile("%s%s/model/%s.wrl" % (parent,folder, folder)):
            copyfile("%s%s/model/%s.wrl" % (parent,folder, folder), "/Users/jeffwinchell/Desktop/NSF_Research/PFIAir/PFI_Scripts/models/%s.wrl" % folder)
        else:
            print(folder)


