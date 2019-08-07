# Run this python file first with ShapeNetCore.v2 dataset to prepare the training, validation and test sets.

import pandas as pd
from path import Path
import os
import os
import csv
from numpy import in1d, setdiff1d
import binvox_rw
import numpy as np

num_classes = 9
PREFIX = Path('~/ShapeNetClassification/').expand()
# SUFFIX = ".binvox"
SUFFIX = ".npy"
DATASET_Path = Path('~/storage/Dataset/ShapeNetCore.v2/')
filename = "model_normalized.solid"
cvs_in = PREFIX+ "/all.csv"
df = pd.read_csv(cvs_in)
count = 0

object_catagory = df['synsetId'].unique()
object_catagory_chair= [3001627]
object_subcatagory = df['subSynsetId'].unique()
B= setdiff1d(object_subcatagory, object_catagory)
A = in1d(B, object_catagory)
category_frequency =  df.groupby('synsetId').size()
category_frequency_sorted = category_frequency.sort_values(ascending=False)
print (category_frequency_sorted[0:num_classes])
category_9= category_frequency_sorted[0:num_classes]
object_categories_9 = [4379243,3001627, 2691156, 2958343, 4256520, 4090263, 3636649, 4530566, 2828884]
def write_csv(csvfile, record):
    with open(csvfile, 'w', newline = '') as f:
        writer = csv.writer(f)
        writer.writerows([['filenames', 'labels']])
        writer.writerows(record)


for catagory in object_categories_9:
    records = {'train': [], 'val': [], 'test': []}
    for row in df.itertuples():
        if getattr(row, "synsetId") == catagory:
            dirname = DATASET_Path + '{:08d}/{}/models/'.format(getattr(row, "synsetId"), getattr(row,"modelId"))
            name_npy = dirname+filename+".npy"
            name_binvox = dirname+ filename+ ".binvox"
            count = count+1
            if os.path.isfile(name_binvox):
                with open(name_binvox, 'rb') as f:
                    model = binvox_rw.read_as_3d_array(f)
                    model_data = model.data
                    model_dims = model.dims
                    model_data = model_data.reshape(model_dims[0], model_dims[1], model_dims[2], 1)
                    np.save(name_npy, model_data)
                records[getattr(row,"split")].append([name_npy,getattr(row, "synsetId")])
    #write train csv
    csvfile = './data_split/{:08d}'.format(catagory)+'_train_labels.csv'
    write_csv(csvfile, records['train'])

    #write test csv
    csvfile = './data_split/{:08d}'.format(catagory) + '_test_labels.csv'
    write_csv(csvfile, records['test'])

    #write validation csv
    csvfile = './data_split/{:08d}'.format(catagory) + '_validation_labels.csv'
    write_csv(csvfile, records['val'])

