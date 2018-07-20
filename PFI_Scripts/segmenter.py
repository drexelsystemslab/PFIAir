import trimesh

import numpy as np
import os
import argparse
import sys


parser = argparse.ArgumentParser(description='Generate segmentation files from benchmark')
parser.add_argument('modeldir', type=str, help='file directory containing models')
parser.add_argument('segdir', type=str, help='file directory containing segmentations')
parser.add_argument('outputdir', type=str, help='file directory into which to output segmentations')
parser.add_argument('fileformat', type=str, default="stl", nargs='?', help='file directory into which to output segmentations')

args = parser.parse_args(sys.argv[1:])

for model in os.listdir(args.modeldir):
    filename = model.split(".")[0]
    print(model)
    try:
        model = trimesh.load_mesh(os.path.join(args.modeldir,model))
    except(OSError,IOError,ValueError):
        print("Problem with model file")
        raise IOError

    if os.path.exists(os.path.join(args.segdir,filename)):
        for segmentation in os.listdir(os.path.join(args.segdir,filename)):
            seg = np.loadtxt(os.path.join(os.path.join(args.segdir,filename),segmentation),dtype=int)
            segmenation_filename = segmentation.split(".")[0]
            for segment_number in range(np.min(seg), np.max(seg) + 1):
                faces = np.argwhere(seg == segment_number)
                submesh = model.submesh(faces,append=True)
                outputdir = os.path.join(os.path.join(args.outputdir,filename), segmenation_filename)
                if not os.path.exists(outputdir):
                    os.makedirs(outputdir)
                segment_filename = os.path.join(outputdir, "%s_%i.%s"%(segmenation_filename,segment_number,args.fileformat))
                with open(segment_filename, "w") as file:
                    try:
                        submesh.export(file,file_type=args.fileformat)
                    except ValueError as e:
                        raise e
    else:
        raise IOError("Seg directory missing")

