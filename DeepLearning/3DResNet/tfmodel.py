# This code is partially based on  Tensor2Robot Authors' film_resnet_model.py:
# https://github.com/google-research/tensor2robot/blob/master/layers/film_resnet_model.py


import tensorflow as tf
import tensorflow.keras.utils as utils
import csv
import random
import numpy as np
import classID
num_classes = 9

_BATCH_NORM_DECAY = 0.997
_BATCH_NORM_EPSILON = 1e-5
#HELPER FUNCTION
#INITI WEIGHTS

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

    def setup_validationSet(self):
        validation_x = self.test_filenames
        validation_y = self.test_labels
        return np.array([np.load(file_name) for file_name in validation_x]), np.array(validation_y)

def init_weights(shape):
    init_random_dist = tf.truncated_normal(shape, stddev = 0.1)
    return tf.Variable(init_random_dist)

#INIT BIAS
def init_bias(shape):
    init_bias_vals = tf.constant(0.1, shape = shape)
    return tf.Variable(init_bias_vals)


#CONV3D
def conv3d(x,W):
    # x --> [batch,H,W,D,Channels In ]
    # W --> [filter H, filter W, filter D, Channels In, Channels OUT]

    return tf.nn.conv3d(x,W,strides=[1,1,1,1,1], padding='SAME')

#POOLING
def max_pooling(x,st):
    # x --> [batch,H,W,D,1]
    # st --> stride
    return tf.nn.max_pool3d(x,ksize =[1,2,2,2,1], strides=[1,st,st,st,1], padding = 'SAME')

#CONVOLUTIONAL LAYER
def convolutional_layer(input_x, shape):
    W = init_weights(shape)
    b = init_bias([shape[3]])
    return tf.nn.relu(conv3d(input_x,W)+b)

# Normal (Fully Connected)
def normal_full_layer(input_layer,size):
    input_size = int(input_layer.get_shape()[1])
    W = init_weights([input_size, size])
    b = init_bias([size])
    return tf.matmul(input_layer,W) + b

def batch_norm(inputs, training):
  return tf.layers.batch_normalization(
      inputs=inputs, axis=-1,
      momentum=_BATCH_NORM_DECAY, epsilon=_BATCH_NORM_EPSILON, center=True, scale=True, training=training, fused=True)

def fixed_padding(inputs, kernel_size):
    pad_total = kernel_size-1
    pad_beg = pad_total//2
    pad_end = pad_total - pad_beg
    padded_inputs = tf.pad(inputs,[[0,0], [pad_beg, pad_end], [pad_beg, pad_end], [pad_beg, pad_end], [0,0]])
    return padded_inputs


def conv3d_fixed_padding(inputs, filters, kernel_size, strides):
  """Strided 3-D convolution with explicit padding."""
  data_format = 'channels_last'
  if strides > 1:
    inputs = fixed_padding(inputs, kernel_size)

  return tf.layers.conv3d(
      inputs=inputs, filters=filters, kernel_size=kernel_size, strides=strides,
      padding=('SAME' if strides == 1 else 'VALID'), use_bias=False,
      kernel_initializer=tf.variance_scaling_initializer(),
      data_format=data_format)


def bottleneck_block(inputs, filters, training, projection_shortcut, strides,i,j):
    shortcut = inputs
    if projection_shortcut is not None:
        shortcut = conv3d_fixed_padding(inputs= inputs, filters=filters*4, kernel_size=1, strides=strides)


    inputs = conv3d_fixed_padding(inputs, filters=filters, kernel_size= 1, strides = 1 )
    inputs = batch_norm(inputs, training)
    inputs = tf.nn.relu(inputs)
    inputs = conv3d_fixed_padding(inputs, filters =filters, kernel_size=3, strides= strides)
    inputs = batch_norm(inputs, training)
    inputs = tf.nn.relu(inputs)
    inputs = conv3d_fixed_padding(inputs, filters =filters * 4, kernel_size=1, strides= 1)
    inputs = batch_norm(inputs, training)
    inputs = inputs + shortcut
    inputs = tf.nn.relu(inputs)
    name = 'BL_{0}_BottleNeck_{1}'.format(i + 1, j+1)
    inputs = tf.identity(inputs, name)
    return inputs

def block_layer(inputs, filters, blocks, bottleneck, strides, training, name, block_number):

    # Bottleneck blocks end with 4x the number of filters as they start with
    filters_out = filters * 4 if bottleneck else filters

    # def projection_shortcut(inputs):
    #     return conv3d_fixed_padding(inputs= inputs, filters=filters_out, kernel_size=1, strides=1)
    projection_shortcut = True

    # Only the first block per layer_block  uses stride.


    if bottleneck:
        inputs = bottleneck_block(inputs, filters, training, projection_shortcut, strides, block_number,0)
        for j in range(1,blocks):
            inputs = bottleneck_block(inputs, filters, training, None, 1, block_number,j)
    else:
        pass

    return tf.identity(inputs, name)



class Model(object):
    def __init__(self, resnet_size, bottleneck, num_classes, num_filters, kernel_size, conv_stride,
                 first_pool_size, first_pool_stride, block_sizes, block_strides):
        """Args:
        resnet_size =size of resnet model, integer
        bottleneck = only the bottleneck version has been implemented
        num_classes = number of classes in classification
        num_filters:The number of filters to use for the first block layer
                    of the model. This number is then doubled for each subsequent block
                    layer.
        kernel_size: The kernel_size for convolution
        conv_stride: stride size for initial convolution layer
        first_pool_size: Pool size to be used for the first pooling layer
        first_pool_stride: stride size for the first pooling layer.
        block_sizes = A list of containing n values,Each value should be the number of blocks in the i-th set
        block_stride: List of integers for the desired stride size

        """
        self.resnet_size= resnet_size
        self.bottleneck = bottleneck
        self.num_classes = num_classes
        self.num_filters = num_filters
        self.kernel_size = kernel_size
        self.conv_stride = conv_stride
        self.first_pool_size = first_pool_size
        self.first_pool_stride = first_pool_stride
        self.block_sizes = block_sizes
        self.block_strides= block_strides

    def __call__(self, inputs, training):
        '''Args
        inputs: A Tensor representing a batch of input images.
        training: A boolean
        '''
        inputs = conv3d_fixed_padding(inputs=inputs, filters = self.num_filters, kernel_size=self.kernel_size,
                                      strides= self.conv_stride)
        inputs = tf.identity(inputs, 'initial_conv')
        inputs = batch_norm(inputs, training)
        inputs = tf.nn.relu(inputs)
        inputs = tf.identity(inputs,'initial_conv_batchNorm_relu')

        if self.first_pool_size:
            inputs = tf.layers.max_pooling3d(inputs, pool_size=self.first_pool_size, strides=self.first_pool_stride,
                                             padding = 'SAME', data_format= 'channels_last')
            inputs = tf.identity(inputs, 'initial_max_pool')

        for i, num_blocks in enumerate(self.block_sizes):
            num_filters = self.num_filters * (2 ** i)
            inputs = block_layer(inputs = inputs, filters = num_filters, bottleneck=True, blocks = num_blocks,
                                 strides = self.block_strides[i], training = training, name = 'block_layer{}'.format(i + 1), block_number =i)

        axes = [1,2,3]
        inputs = tf.reduce_mean(inputs, axes, keepdims=True)
        inputs = tf.identity(inputs, 'final_reduce_mean')
        inputs = tf.squeeze(inputs, axes)
        inputs = tf.layers.dense(inputs = inputs, units = self.num_classes)
        inputs = tf.identity(inputs, 'final_dense')
        return inputs




#PLACEHOLDERS
ds = DatasetHelper()
ds.setup_images()
print (ds.length)
print (ds.train_labels.shape)
print (ds.i)

inputs = tf.placeholder(tf.float32, shape = [None, 128,128,128,1])
y_true = tf.placeholder(tf.int32, shape = [None, num_classes])

#Model
resnet_model = Model(resnet_size=50, bottleneck=True, num_classes=num_classes, num_filters=64,kernel_size=7,conv_stride=2,
                     first_pool_size=3, first_pool_stride=2, block_sizes=[3,4,6,3], block_strides=[1,2,2,2])


logits = resnet_model(inputs, training = True)
logits = tf.cast(logits, tf.float32)
predictions = {
      'classes': tf.argmax(logits, axis=1),
      'probabilities': tf.nn.softmax(logits, name='softmax_tensor')
}

cross_entropy = tf.losses.softmax_cross_entropy(onehot_labels=y_true, logits = logits)
tf.identity(cross_entropy, name='cross_entropy')
tf.summary.scalar('cross_entropy', cross_entropy)

optimizer = tf.train.AdadeltaOptimizer(learning_rate=0.001)
train = optimizer.minimize(cross_entropy)
init = tf.global_variables_initializer()

steps = 20000
saver = tf.train.Saver()


# def __getitem__(self, idx, batch_size):
#     batch_x = self.filenames[idx * self.batch_size:(idx + 1) * self.batch_size]
#     batch_y = self.labels[idx * self.batch_size:(idx + 1) * self.batch_size]
#
#     return np.array([np.load(file_name) for file_name in batch_x]), np.array(batch_y)

with tf.Session() as sess:
    sess.run(init)
    for i in range(steps):
        batch_x, batch_y = ds.next_batch(8)
        sess.run(train, feed_dict={inputs:batch_x, y_true:batch_y})
        if i%100 ==0:
            print('Currently on step {}'.format(i))
            print('Accuracy is:')
            matches = tf.equal(tf.argmax(logits, 1), tf.argmax(y_true, 1))
            acc = tf.reduce_mean(tf.cast(matches, tf.float32))
            print(sess.run(acc, feed_dict={inputs:batch_x, y_true:batch_y }))
            print('\n')

        if i%3000 ==0:
            j=i//3000
            saver.save(sess,'model/resnet_model_1_iter_3000_{}.ckpt'.format(j))
    saver.save(sess,'model/resnet_model_1.ckpt')


validation_x, validation_y = ds.setup_validationSet()
print('Accuracy for validation set is: ')
matches = tf.equal(tf.argmax(logits, 1), tf.argmax(y_true, 1))
acc = tf.reduce_mean(tf.cast(matches, tf.float32))
print(sess.run(acc, feed_dict={inputs:validation_x, y_true:validation_y}))
print('\n')


