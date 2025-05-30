#ifndef MATRIX_H
#define MATRIX_H

#include <memory>
#include "Types.h"

typedef double* Matrix_t;

namespace nucare {

namespace math {

#define MAT_2D_VAL(a, COLS, row, col) *(a + row * COLS + col)

class Matrix {
private:
    std::shared_ptr<double[]> mData;

    static constexpr int ROWS = 3;
    static constexpr int COLS = 3;
    static constexpr int SIZE = ROWS * COLS;
public:
    Matrix();
    Matrix(Matrix_t data);
    Matrix(std::shared_ptr<double[]>&& data);
    Matrix(const Matrix& m);
    Matrix(const Matrix&& m);
    void revert(Matrix& out);
    double determine();
    inline double* data() { return mData.get(); }
    double* at(const int& row, const int& col) { return Matrix::at(mData.get(), row, col); }
    double value(const int& row, const int& col) const { return *(mData.get() + row * ROWS + col); }
    double* multiply(const double* vector, const int& length);
    double* operator *(const Matrix& m);

    Vec1D multiply(const Vec1D& vector);


    std::string toString();

    Matrix& operator=(const Matrix& m);
    Matrix& operator=(Matrix&& m) noexcept;

    static inline double* at(Matrix_t m, const int row, const int col) { return m + row * ROWS + col; }
};

void TransposeMatrix(double* in, double* out, int nRow, int mCol);

void Multi2Matrix(const double* A, const int nRowA, const int mColA,
                  const double* B, const int nRowB, const int mColB,
                  double* Res);
void InverseMatrix(double* out, const double* A, const int n);
void InverseMatrix(double** out, const double** A, const int n);
void InverseMatrix(Vec2D& out, const Vec2D& A, const int n);
void MultiMatrix2Dby1D(const double* A, const int nRowA, const int mColA,
                       const double* B, const int nRowB, double* Res);
void MultiMatrix2Dby1D(const Vec2D& A, const Vec1D& B, Vec1D& Res);

void AddMatrix1D(const double* A, const double* B, const int N, double* Res);
double Rsquare_Fit(const double* y, const double* f0, const int n);

}

}


#endif // MATRIX_H
