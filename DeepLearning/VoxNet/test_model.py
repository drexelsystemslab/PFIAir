from tensorflow.keras.models import load_model
import random
import csv
import tensorflow.keras.utils as utils
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import shapenet10

num_classes = 10

with open('test_labels.csv') as csvfile:
    readCSV = csv.reader(csvfile, delimiter=',')
    next(readCSV)
    filelist = list(readCSV)
    random.shuffle(filelist)
    test_filelist = np.array(filelist)
    test_filenames = test_filelist[:,0]
    test_labels = test_filelist[:,1]
    test_labels = test_labels.astype(int)
    test_labels = utils.to_categorical(test_labels, num_classes = num_classes)

model = load_model('my_model.h5')

test_filename = input("Select an integer number for test file name? ")
test_filename = int(test_filename)
test_image = np.load(test_filenames[test_filename])
plt_test = test_image[:,:,:,0]
print(plt_test.shape)
print (test_filenames[test_filename])

fig = plt.figure()
ax = fig.gca(projection='3d')
ax.voxels(plt_test, edgecolor='k')
plt.show()

test_image = test_image[None,:]
print(test_image.shape)
scores = model.predict(test_image)
print (scores)
class_id = np.argmax(scores)+1
print(shapenet10.class_id_to_name[str(class_id)])