import tensorflow.keras.utils as utils
import csv
import random
import numpy as np
import classID
num_classes = 9



class DatasetHelper(object):

    def __init__(self):
        self.i = 0
        self.train_filenames = None
        self.train_labels = None
        self.test_filenames = None
        self.test_labels = None
        self.length = None

    def setup_images(self):
        with open('train_labels.csv') as csvfile:
            readCSV = csv.reader(csvfile, delimiter=',')
            next(readCSV)
            filelist = list(readCSV)
            random.shuffle(filelist)
            train_filelist = np.array(filelist)
            self.length = train_filelist.shape[0]
            self.train_filenames = train_filelist[:, 0]
            train_labels = train_filelist[:, 1]
            train_labels = classID.class_name_to_id(list(train_labels))
            train_labels = np.array(train_labels)
            self.train_labels = train_labels.astype(int)
            self.train_labels = utils.to_categorical(train_labels, num_classes=num_classes)

        with open('val_labels.csv') as csvfile:
            readCSV = csv.reader(csvfile, delimiter=',')
            next(readCSV)
            filelist = list(readCSV)
            random.shuffle(filelist)
            test_filelist = np.array(filelist)
            self.test_filenames = test_filelist[:, 0]
            test_labels = test_filelist[:, 1]
            test_labels = classID.class_name_to_id(list(test_labels))
            test_labels = np.array(test_labels)
            test_labels = test_labels.astype(int)
            self.test_labels = test_labels.astype(int)
            self.test_labels = utils.to_categorical(test_labels, num_classes=num_classes)

    def next_batch(self, batch_size = 32):
        batch_x = self.train_filenames[self.i:self.i + batch_size]
        batch_y = self.train_labels[self.i:self.i + batch_size]
        self.i = (self.i + batch_size) % self.length
        return np.array([np.load(file_name) for file_name in batch_x]), np.array(batch_y)

    def setup_validationSet(self,start, number):
        validation_x = self.test_filenames[start:start+number]
        validation_y = self.test_labels[start:start+number]
        return np.array([np.load(file_name) for file_name in validation_x]), np.array(validation_y)