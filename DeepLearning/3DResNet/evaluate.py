import tensorflow as tf
import DatasetHelper

num_classes = 9
ds = DatasetHelper.DatasetHelper()
ds.setup_images()
print (ds.test_filenames.shape)
#validation_x, validation_y = ds.setup_validationSet(number = 70)
tf.reset_default_graph()

with tf.Session() as sess:

    # Restore the model
    saver = tf.train.import_meta_graph('model/resnet_model_0.ckpt.meta')
    saver.restore(sess, tf.train.latest_checkpoint('model/'))
    graph =tf.get_default_graph()
    #print(graph.get_operations())
    inputs = graph.get_tensor_by_name("Placeholder:0")
    y_true = graph.get_tensor_by_name("Placeholder_1:0")
    logits = graph.get_tensor_by_name("final_dense:0")
    logits = tf.cast(logits, tf.float32)
    accuracy=[]
    #print('Accuracy for validation set is: ')
    for i in range(0,3351,50):
        validation_x, validation_y = ds.setup_validationSet(i,50)
        matches = tf.equal(tf.argmax(logits, 1), tf.argmax(y_true, 1))
        acc = tf.reduce_mean(tf.cast(matches, tf.float32))
        accuracy.append(float(sess.run(acc, feed_dict={inputs: validation_x, y_true: validation_y})))
    print('accuracy for every 50 in validation set: ')
    print(accuracy)
    print('Accuracy for validation set is: ')
    ave = sum(accuracy)/float(len(accuracy))
    print(ave)
    print('\n')
