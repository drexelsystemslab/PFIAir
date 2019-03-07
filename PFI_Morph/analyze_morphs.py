#!/usr/bin/env python
from __future__ import print_function
import os

import pandas
import numpy
import matplotlib.pyplot as plt
from sklearn.linear_model import LinearRegression
from sklearn.model_selection import train_test_split
from sklearn.model_selection import cross_val_predict
from sklearn.model_selection import cross_val_score
from sklearn.preprocessing import StandardScaler
from sklearn.decomposition import PCA
from sklearn.svm import SVR
from sklearn.pipeline import make_pipeline

numpy.set_printoptions(precision=3, suppress=True)

from pfimorph.config import config

ANALYTICS_DIR = config.get('output', 'analytics_dir')

def get_data_frame():
    """
    Read the CSV file outputted by post_process.py
    """
    csv_file = os.path.join(ANALYTICS_DIR, 'morph_table.csv')
    df = pandas.read_csv(csv_file)
    return df.set_index(['source', 'target'])

def get_expected_dissimilarity():
    """
    Get the expected dissimilarity table created in ground_truth.py
    """
    csv_file = os.path.join(ANALYTICS_DIR, 'expected_dissimilarity.csv')
    df = pandas.read_csv(
        csv_file,
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

    print(X_train.shape)

    pipeline = make_pipeline(
        StandardScaler(), LinearRegression())
    pipeline.fit(X_train, Y_train)
    predicted = pipeline.predict(X_test)

    model = pipeline.named_steps['linearregression'].coef_
    intercept = pipeline.named_steps['linearregression'].intercept_
    print(model)
    print(intercept)

    plot_file = os.path.join(ANALYTICS_DIR, 'linreg.png')
    plot_reg(
        plot_file,
        'Dissimilarity: (LinReg) Predicted vs Actual', 
        predicted, 
        Y_test)
    print("linreg R^2", pipeline.score(X_test, Y_test))

def linreg_cross_validation(X, Y):
    """
    Perform linear regression on arrays X and Y. This is done with the
    traditional test/train split
    """

    pipeline = make_pipeline(
        StandardScaler(), LinearRegression())

    predictions = cross_val_predict(pipeline,  X, Y, cv=10)
    scores = cross_val_score(pipeline, X, Y, cv=10)

    plot_file = os.path.join(ANALYTICS_DIR, 'linreg_cv.png')
    plot_reg(
        plot_file,
        'Dissimilarity: (LinReg Cross-Validated) Predicted vs Actual', 
        predictions, 
        Y)
    print("Average Cross-Validated Score", scores.mean())


    '''
    pipeline.fit(X_train, Y_train)
    predicted = pipeline.predict(X_test)

    model = pipeline.named_steps['linearregression'].coef_
    intercept = pipeline.named_steps['linearregression'].intercept_
    print(model)
    print(intercept)

    '''

def linreg(X, Y):
    """
    Perform linear regression on all the data and return the model
    coefficients as well as a vector of predictions for each model

    This has a danger of overfitting the data, but we're out of time on
    this project and we just wanted to see how much of a correlation between
    morph data and expected results there is.
    """
    pipeline = make_pipeline(
        StandardScaler(), LinearRegression())
    pipeline.fit(X, Y)

    predictions = pipeline.predict(X)
    model = pipeline.named_steps['linearregression'].coef_
    intercept = pipeline.named_steps['linearregression'].intercept_

    return model, intercept, predictions 

def plot_expected_vs_actual(predictions, Y):
    """
    Plot a graph of regression results
    """
    plot_fname = os.path.join(ANALYTICS_DIR, 'linreg_results.png')
    plot_reg(
        plot_fname,
        'Dissimilarity: Predicted vs Actual',
        predictions,
        Y)

def plot_heatmap(n_by_n_table, predictions):
    """
    Plot a heatmap of the results
    """

def display_model(model, intercept, column_labels):
    """
    Pretty print the model coefficients labeled by feature name, sorted
    descending by magnitude of the coefficient. This should give a rough
    view of what features have the biggest impact on the model

    Also, note that the coefficients are computed after standardizing
    the data to have mean 0 and standard deviation 1, so they do not 
    correspond to weights of the raw features.
    """
    table = pandas.DataFrame(
        data=model,
        index=column_labels,
        columns=["Coefficient"])
    table['Magnitude'] = table['Coefficient'].abs()
    by_magnitude = table.sort_values(by='Magnitude', ascending=False)
    print(by_magnitude)

    # Also save it in a csv file
    csv_file = os.path.join(ANALYTICS_DIR, 'model_coeffs.csv')
    by_magnitude.to_csv(csv_file)

def main():
    data = get_data_frame()
    expected = get_expected_dissimilarity()
    merged = pandas.merge(data, expected, left_index=True, right_index=True)

    X, Y = to_arrays(merged)
    model, intercept, predictions = linreg(X, Y)
    plot_expected_vs_actual(predictions, Y)
    plot_heatmap(merged, predictions)
    display_model(model, intercept, merged.columns[:-1]) 

if __name__ == '__main__':
    main()
