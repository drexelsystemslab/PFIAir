#ifndef PCA_CALCULATOR_H
#define PCA_CALCULATOR_H
#include <eigen3/Eigen/Dense>
class PCACalculator {
    // Covariance matrix = X * X^T where X is the vertices as column vectors
    Eigen::Matrix3d covariance;

    // Eigenvectors and Eigenvalues (at least the real components of each)
    Eigen::Vector3d eigenvalues;
    Eigen::Matrix3d eigenvectors;

    /**
     * Populate the covariance matrix = X * X^T
     * The vertices are in homogeneous coordinates, so 
     * ignore the w component.
     */
    void compute_covariance(const Eigen::MatrixXd& vertices);

    /**
     * Perform eigenvector decomposition on the covariance matrix.
     * store eigenvalues and eigenvectors as a matrix and a vector
     */
    void perform_eigen_decomposition();

    /**
     * Sort the eigenvectors and eigenvalues ascending by
     * eigenvalue. This updates the two fields eigenvalues and eigenvectors
     *
     * When finished, the eigenvectors will be sorted like this
     *
     * x -> min eigenvalue
     * y -> middle eigenvalue
     * z -> max eigenvalue
     */
    void sort_eigenvectors();
    /**
     * Rearrange the sorted eigenvector matrix and turn it into
     * a 4x4 rotation matrix for use with homogeneous coords.
     *
     * The matrix will have the form:
     *
     * [xx xy xz 0]
     * [yx yy yz 0] = [E^T 0] where E is the sorted eigenvector matrix.
     * [zx zy zz 0]   [ 0  1]
     * [0  0  0  1]
     * 
     * Where the three vectors are from the *sorted* eigenvectors.
     *
     * Intuition behind this:
     * The eigenvector matrix is a rotation that aligns 
     *      (x, y, z) -> eigenvector basis
     * We want the inverse transform that aligns
     *      eigenvector basis -> (x, y, z)
     * so we compute the *inverse* of the rotation matrix. For rotation
     * matrices, the inverse is equal to the transpose.
     */
    Eigen::Matrix4d create_rotation_matrix();
public:
    static const int X = 0;
    static const int Y = 1;
    static const int Z = 2;
    static const int W = 4;
    /**
     * Entry point for the calculator. Give it some vertices
     * and it will produce a rotation matrix.
     */
    Eigen::Matrix4d perform_pca(const Eigen::MatrixXd& vertices);
};

#endif
