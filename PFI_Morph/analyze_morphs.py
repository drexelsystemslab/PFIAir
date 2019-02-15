#!/usr/bin/env python
from __future__ import print_function

import pandas
import numpy
from sklearn.preprocessing import StandardScaler
from sklearn.decomposition import PCA
from sklearn.pipeline import make_pipeline

numpy.set_printoptions(precision=3, suppress=True)

def get_data_frame():
    """
    Read the CSV file outputted by post_process.py
    """
    df = pandas.read_csv('output/test_table.csv')
    return df.set_index(['source', 'target'])

def get_expected_dissimilarity():
    """
    Get the expected dissimilarity table created in ground_truth.py
    """
    df = pandas.read_csv(
        'output/expected_dissimilarity.csv', 
        header=None,
        names=['source', 'target', 'expected_dissimilarity'])
    return df.set_index(['source', 'target'])

def perform_pca(data):
    """
    Perform principal component analysis to find the combination
    of features that is most important
    """
    arr = data.values

    pipeline = make_pipeline(StandardScaler(), PCA())
    pipeline.fit(arr)

    pca = pipeline.named_steps['pca']
    print("PCA Components:")
    print(pca.components_)
    print("Explained variance")
    print(pca.explained_variance_)
    

def main():
    data = get_data_frame()
    #perform_pca(data)

    expected = get_expected_dissimilarity()

    merged = pandas.merge(data, expected, left_index=True, right_index=True)

if __name__ == '__main__':
    main()
