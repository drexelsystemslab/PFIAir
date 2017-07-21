import to_stl
from split import split
import os
import requests
from io import BytesIO
#path = "/Users/jeffwinchell/Desktop/NSF_Research/PFIAir/PFI_Scripts/models/"
#for file in os.listdir(path):
  #  id = os.path.splitext(file)[0] + ".stl"
  #  to_stl.pickFileType(path + file, "STL_models/" + id)
#path = "/Users/jeffwinchell/Desktop/NSF_Research/PFIAir/PFI_Scripts/STL_models/"
#for file in os.listdir(path):
 #   no_extension = file.split('.')[0]
  #  split(path, no_extension)
path = "/Users/jeffwinchell/Desktop/NSF_Research/PFIAir/PFI_Scripts/Split_STL_models/"
i = 0
for file in os.listdir(path):
    name = os.path.splitext(file)[0]
    requests.post("http://127.0.0.1:8000/model", data={"name":name,"file":BytesIO(file)})
    i += 1
    print(i)
print("number of files posted: " + i)