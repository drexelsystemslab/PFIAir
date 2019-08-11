from subprocess import call

def pickFileType (url, target_url):
    extension = url.split('.')[-1]
    if extension == ("3ds" or "3DS"):
        threeds_to_stl(url, target_url)
    elif extension == ("obj" or "OBJ"):
        obj_to_stl(url, target_url)
    elif extension == ("wrl" or "WRL"):
        wrl_to_stl(url,target_url)
    elif extension == ("stl" or "STL"):
        pass
    else:
        raise Exception('file type not supported: ' + url)

def threeds_to_stl (url, target_url):
    print("3ds: " + url)
    call(["blender", "--background",
          "--python",
          "blender3dsToStl.py",
          "--",
          url,
          target_url])

def obj_to_stl (url, target_url):
    print("Obj: " + url)
    call(["blender", "--background",
          "--python",
          "blenderObjToStl.py",
          "--",
          url,
          target_url])

def wrl_to_stl (url, target_url):
    print("Wrl:" + url)
    call(["blender", "--background",
          "--python",
          "blenderWrlToStl.py",
          "--",
          url,
          target_url])

