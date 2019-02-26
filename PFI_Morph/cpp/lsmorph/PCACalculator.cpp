#include "PCACalculator.h"
#include <eigen3/Eigen/Eigenvalues>
#include <algorithm>
#include <iostream>

const int PCACalculator::X;
const int PCACalculator::Y;
const int PCACalculator::Z;
const int PCACalculator::W;

void PCACalculator::compute_covariance(const Eigen::MatrixXd& vertices) {
    // Remove the homogeneous coordinates
    int N = vertices.cols();
    Eigen::MatrixXd data = vertices.block(0, 0, 3, N);

    // Covariance matrix = X * X^T when the data is stored as column vectors
    covariance = data * data.transpose();
}

void PCACalculator::perform_eigen_decomposition() {
    Eigen::EigenSolver<Eigen::Matrix3d> solver(covariance);
    eigenvalues = solver.eigenvalues().col(0).real();
    eigenvectors = solver.eigenvectors().real();
}

void PCACalculator::sort_eigenvectors() {
    // Sort both the eigenvector matrix and eigenvalue vector
    // so they go from lowest eigenvalue to greatest
    if (eigenvalues(X) > eigenvalues(Y)) {
        std::swap(eigenvalues(X), eigenvalues(Y));
        eigenvectors.col(X).swap(eigenvectors.col(Y));
    }
    if (eigenvalues(X) > eigenvalues(Z)) {
        std::swap(eigenvalues(X), eigenvalues(Z));
        eigenvectors.col(X).swap(eigenvectors.col(Z));
    }
    if (eigenvalues(Y) > eigenvalues(Z)) {
        std::swap(eigenvalues(Y), eigenvalues(Z));
        eigenvectors.col(Y).swap(eigenvectors.col(Z));
    }
}

Eigen::Matrix4d PCACalculator::create_rotation_matrix() {
    // Size of the matrix
    const int N = 3;

    // Allocate a 4x4 transformation matrix
    Eigen::Matrix4d results(N + 1, N + 1);

    // The first 3x3 section of the matrix is the inverse rotation
    // which is simply eigenvectors.transpose()
    results.block<N, N>(0, 0) = eigenvectors.transpose();

    // Add another row and column for the homogeneous coordinates
    // Everything is set to zero except the bottom right element which is
    // 1. In the end, the matrix has the structure:
    // [E^T 0]
    // [0   1]
    results.block<1, N>(N, 0).setZero();
    results.block<N, 1>(0, N).setZero();
    results(N, N) = 1;
    return results;
}

Eigen::Matrix4d PCACalculator::perform_pca(const Eigen::MatrixXd& vertices) {
    compute_covariance(vertices);
    perform_eigen_decomposition();
    sort_eigenvectors();
    return create_rotation_matrix();
}
