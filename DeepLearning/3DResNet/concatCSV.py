import pandas as pd
from os import listdir
from os.path import isfile, join
from sklearn.utils import shuffle

train_dir_path = "~/ShapeNetClassification/data_split/train"
test_dir_path = "~/ShapeNetClassification/data_split/test"
val_dir_path = "~/ShapeNetClassification/data_split/val"
train_files_list = [join(train_dir_path,f) for f in listdir(train_dir_path) if isfile(join(train_dir_path,f))]
test_files_list = [join(test_dir_path,f) for f in listdir(test_dir_path) if isfile(join(test_dir_path,f))]
val_files_list = [join(val_dir_path,f) for f in listdir(val_dir_path) if isfile(join(val_dir_path,f))]
print(train_files_list)

train_combined_csv = pd.concat( [ pd.read_csv(f) for f in train_files_list ] )
train_combined_csv_shuffled = shuffle(train_combined_csv)
train_combined_csv_shuffled.to_csv( "train_labels.csv", index=False )


test_combined_csv = pd.concat([pd.read_csv(f) for f in test_files_list ])
test_combined_csv_shuffled = shuffle(test_combined_csv)
test_combined_csv_shuffled.to_csv("test_labels.csv", index=False )

val_combined_csv = pd.concat([pd.read_csv(f) for f in val_files_list ])
val_combined_csv_shuffled = shuffle(val_combined_csv)
val_combined_csv_shuffled.to_csv("val_labels.csv", index=False )