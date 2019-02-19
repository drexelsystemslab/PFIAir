#!/usr/bin/env python
from __future__ import print_function

import pandas
import numpy
import matplotlib.pyplot as plt
from sklearn.linear_model import LinearRegression
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.decomposition import PCA
from sklearn.svm import SVR
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

def to_arrays(merged):
    """
    Take the merged data frame and turn it into two numpy arrays
    X and Y
    """
    arr = merged.values
    X = arr[:, :-1]
    Y = arr[:, -1]
    return X, Y

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

def plot_reg(fname, title, predicted, actual):
    """
    Save a diagram of the results of a linear regression
    """
    plt.figure()
    plt.scatter(actual, predicted)
    plt.axis('scaled')
    plt.axis([0, 1, 0, 1])
    plt.xlabel('Actual')
    plt.ylabel('Predicted')
    plt.title(title)
    plt.savefig(fname)

def linreg(X, Y):
    """
    Perform linear regression on arrays X and Y. This is done with the
    traditional test/train split
    """
    # Split into training and testing data
    X_train, X_test, Y_train, Y_test = train_test_split(
        X, Y, test_size=0.25, random_state=0)

    pipeline = make_pipeline(StandardScaler(), LinearRegression())
    pipeline.fit(X_train, Y_train)
    predicted = pipeline.predict(X_test)
    plot_reg(
        'output/linreg.png', 
        'LinReg', 
        predicted, 
        Y_test)
    print("linreg R^2", pipeline.score(X_test, Y_test))

def linreg_pca(X, Y, n):
    """
    Perform linear regression using PCA analysis to reduce the feature
    set down to n features for graphing purposes
    """
    X_train, X_test, Y_train, Y_test = train_test_split(
        X, Y, test_size = 0.25, random_state=0)
    
    pipeline = make_pipeline(
        StandardScaler(), PCA(n_components=n), LinearRegression())
    pipeline.fit(X_train, Y_train)
    predicted = pipeline.predict(X_test)
    plot_reg(
        'output/linreg_pca{}.png'.format(n), 
        'LinReg with PCA ({} Components)'.format(n), 
        predicted, 
        Y_test)

def svr(X, Y, deg):
    X_train, X_test, Y_train, Y_test = train_test_split(
        X, Y, test_size = 0.25, random_state=0)

    pipeline = make_pipeline(
        StandardScaler(), SVR(kernel='poly', degree=deg))
    pipeline.fit(X_train, Y_train)
    predicted = pipeline.predict(X_test)
    plot_reg(
        'output/svr{}.png'.format(deg), 
        'SVR (Polynomial degree={})'.format(deg), 
        predicted, 
        Y_test) 
    print("svr R^2", pipeline.score(X_test, Y_test))

def svr_pca(X, Y, deg, n):
    X_train, X_test, Y_train, Y_test = train_test_split(
        X, Y, test_size = 0.25, random_state=0)

    pipeline = make_pipeline(
        StandardScaler(), PCA(n_components=n), SVR(kernel='poly', degree=deg, gamma='auto'))
    pipeline.fit(X_train, Y_train)
    predicted = pipeline.predict(X_test)
    plot_reg(
        'output/svr{}_pca{}.png'.format(deg, n), 
        'SVR (Polynomial degree={}) with PCA ({})'.format(deg, n), 
        predicted, 
        Y_test) 
    print("svr pca R^2", pipeline.score(X_test, Y_test))

def main():
    data = get_data_frame()
    expected = get_expected_dissimilarity()
    merged = pandas.merge(data, expected, left_index=True, right_index=True)

    X, Y = to_arrays(merged)
    linreg(X, Y)
    #linreg_pca(X, Y, 2)
    #linreg_pca(X, Y, 3)
    svr(X, Y, 3)
    #svr(X, Y, 4)
    #svr_pca(X, Y, 3, 2)
    #svr_pca(X, Y, 4, 3)

if __name__ == '__main__':
    main()
