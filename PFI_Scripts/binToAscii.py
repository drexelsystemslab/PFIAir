import sys
import trimesh
import os
import fileinput


def binToAscii(inpath,outpath):
	try:
		try:
			model = trimesh.load_mesh(inpath)
			try:
				with open(outpath, "w+") as file:
					model.export(file,'stl_ascii')
			except(IOError):
				raise OSError("unable to write file")
    		except(IOError):
			raise OSError("stl file missing")
		except(IndexError):
			raise OSError("invalid stl")
		except(UnicodeDecodeError):
			raise OSError("invalid encoding on stl")
	except OSError as e:
		print(inpath + ": " + e.message)
if(len(sys.argv) < 3):
	print("Usage: binToAscii.py <input_file or directory> <output_file or directory>")
else:
	if(os.path.isfile(sys.argv[1])):
		binToAscii(sys.argv[1],sys.argv[2])
	elif(os.path.isdir(sys.argv[1])):
		for filename in os.listdir(sys.argv[1]):
			binToAscii(os.path.join(sys.argv[1],filename),os.path.join(sys.argv[2],filename))
	else:
		print("input is not file or directory")
