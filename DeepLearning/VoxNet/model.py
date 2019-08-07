import numpy as np
import tensorflow as tf
from tensorflow.keras.layers import Input, Dense, Flatten, Conv3D, MaxPooling3D
from tensorflow.keras.models import Sequential, Model
from tensorflow.keras.initializers import glorot_uniform
import tensorflow.keras.utils as utils
import tensorflow.keras.backend as K
K.set_image_data_format('channels_last')
K.set_learning_phase(1)
import random
import csv
from sequence import MY_Generator
import matplotlib.pyplot as plt

num_classes=10
input_shape=(32, 32, 32, 1)
model = Sequential()
model.add(Conv3D(32, (5, 5, 5), strides=(2, 2, 2), data_format="channels_last", name='conv1',
                kernel_initializer=glorot_uniform(seed=0) ,input_shape= input_shape))
model.add(Conv3D(32, (3, 3, 3), strides=(1, 1, 1), data_format="channels_last", name='conv2',
             kernel_initializer=glorot_uniform(seed=0)))
model.add( MaxPooling3D(pool_size=(2, 2, 2), strides=None, padding='valid'))
model.add(Flatten())
model.add(Dense(128, activation='softmax', name='fc3', kernel_initializer=glorot_uniform(seed=0)))
model.add(Dense(num_classes, activation='relu', name='fc3_' + str(num_classes), kernel_initializer=glorot_uniform(seed=0)))

model.compile(optimizer='rmsprop', loss='categorical_crossentropy', metrics=['accuracy'])




with open('train_labels.csv') as csvfile:
    readCSV = csv.reader(csvfile, delimiter=',')
    next(readCSV)
    filelist = list(readCSV)
    random.shuffle(filelist)
    train_filelist = np.array(filelist)
    train_filenames = train_filelist[:,0]
    train_labels = train_filelist[:,1]
    train_labels = train_labels.astype(int)
    train_labels = utils.to_categorical(train_labels, num_classes = num_classes)

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

batch_size = 512
num_epochs = 10
my_training_batch_generator = MY_Generator(train_filenames, train_labels, batch_size)
my_test_batch_generator = MY_Generator(test_filenames, test_labels, batch_size)
my_validation_batch_generator = MY_Generator(test_filenames, test_labels, batch_size)

num_training_samples = train_filenames.shape[0]
num_validation_samples = test_filenames.shape[0]

print (num_training_samples // batch_size)
history = model.fit_generator(generator=my_training_batch_generator,
                                          steps_per_epoch=(num_training_samples // batch_size),
                                          epochs=num_epochs,
                                          verbose=1,
                                          validation_data=None,
                                          validation_steps=None,
                                          use_multiprocessing=True)

print(history.history.keys())

#summerize history for accuracy
plt.plot(history.history['acc'])
plt.title('model accuracy')
plt.ylabel('accuracy')
plt.xlabel('epoch')
plt.show()
plt.plot(history.history['loss'])
plt.title('model loss')
plt.ylabel('loss')
plt.xlabel('epoch')
plt.show()

model.save('my_model.h5')
json_string = model.to_json()


loss, accuracy = model.evaluate_generator(generator=my_test_batch_generator,
                                          use_multiprocessing=True)
print('Accuracy: ', accuracy)
print('Loss: ', loss)


