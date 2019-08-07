# Generate train_labels.csv, test_labels.csv and 'validation_labels.csv' based on
# modelnet10 dataset and directory structrue.
# Run this file first before building the model and training it.
import scipy.io
import numpy as np
from path import Path
import shapenet10
import io
import tarfile
import time
import zlib
import os
import csv

PREFIX = Path('~/ObjectClassification/data/').expand()
SUFFIX = '.npy'
class NpyTarWriter(object):
    def __init__(self, fname):
        self.tfile = tarfile.open(fname, 'w|')

    def add(self, arr, name):

        sio = io.BytesIO()
        np.save(sio, arr)
        zbuf = zlib.compress(sio.getvalue())
        sio.close()

        zsio = io.BytesIO(zbuf)
        tinfo = tarfile.TarInfo('{}{}{}'.format(PREFIX, name, SUFFIX))
        tinfo.size = len(zbuf)
        tinfo.mtime = time.time()
        zsio.seek(0)
        self.tfile.addfile(tinfo, zsio)
        zsio.close()

    def close(self):
        self.tfile.close()


def write(records, fname):
    writer = NpyTarWriter(fname)
    for (classname, instance, rot, fname) in records:
        class_id = int(shapenet10.class_name_to_id[classname])
        name = '{:03d}.{}.{:03d}'.format(class_id, instance, rot)
        name =  PREFIX + name + SUFFIX
        arr = scipy.io.loadmat(fname)['instance'].astype(np.uint8)
        arrpad = np.zeros((32,)*3, dtype=np.uint8)
        arrpad[1:-1,1:-1,1:-1] = arr
        np.save(name, arrpad)
        #writer.add(arrpad, name)
    #writer.close()


base_dir = Path('~/ObjectClassification/ModelNet10VolumetricData').expand()
records = {'train': [], 'test': []}
labels =[]
for fname in sorted(base_dir.walkfiles('*.mat')):
    print (fname)
    elts = fname.splitall()
    instance_rot = Path(elts[-1]).stripext()
    instance = instance_rot[:instance_rot.rfind('_')]
    rot = int(instance_rot[instance_rot.rfind('_')+1:])
    split = elts[-2]
    classname = elts[-4].strip()
    class_id = int(shapenet10.class_name_to_id[classname])-1
    # records[split].append((classname, instance, rot, fname))
    #name = '{:03d}.{}.{:03d}'.format(class_id, instance, rot)
    name = PREFIX + '/{}/{}/{}.{:03d}'.format(split,classname,instance, rot)+ SUFFIX
    print(name)
    #name = PREFIX + name + SUFFIX
    if not os.path.exists(os.path.dirname(name)):
         os.makedirs(os.path.dirname(name))

    arr = scipy.io.loadmat(fname)['instance'].astype(np.uint8)
    arrpad = np.zeros((32,) * 3, dtype=np.uint8)
    arrpad[1:-1, 1:-1, 1:-1] = arr
    arrpad = arrpad.reshape(32, 32, 32, 1)
    np.save(name, arrpad)
    records[split].append([name, class_id])


with open('train_labels.csv', 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerows([['filenames','labels']])
    writer.writerows(records['train'])

with open('test_labels.csv', 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerows([['filenames', 'labels']])
    writer.writerows(records['test'])

with open('validation_labels.csv', 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerows([['filenames', 'labels']])
    writer.writerows(records['test'])
