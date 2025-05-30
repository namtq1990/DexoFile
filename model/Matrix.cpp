#include "Matrix.h"
#include "nc_exception.h"
#include <cstring>
#include <sstream>
using namespace std;
using namespace nucare::math;

Matrix::Matrix()
{
    double* data = new double[9] {
            0, 0, 0,
            0, 0, 0,
            0, 0, 0
    };
    mData = shared_ptr<double[]>(data);
}

Matrix::Matrix(Matrix_t data)
{
    mData = shared_ptr<double[]>(data);
}

Matrix::Matrix(std::shared_ptr<double[]> &&data)
{
    mData = data;
}

Matrix::Matrix(const Matrix &m)
{
    double* data = new double[SIZE];
    memcpy(data, m.mData.get(), SIZE * sizeof(double));
    mData = shared_ptr<double[]>(data);
}

Matrix::Matrix(const Matrix &&m)
{
    mData = m.mData;
}

double Matrix::determine() {
    return value(0, 0) * value(1, 1) * value(2, 2)
            + value(2, 0) * value(0, 1) * value(1, 2)
            + value(1, 0) * value(2, 1) * value(0, 2)
            - value(0, 2) * value(1, 1) * value(2, 0)
            - value(0, 0) * value(1, 2) * value(2, 1)
            - value(1, 0) * value(0, 1) * value(2, 2);
}

double *Matrix::multiply(const double *vector, const int &length)
{
    if (COLS != length)
        NC_THROW_ARG_ERROR("Can't multiply vector different size");
    double* rs = new double[ROWS];
    for (int i = 0; i < ROWS; i++) {
        rs[i] = 0;

        for (int j = 0; j < COLS; j++) {
            rs[i] += value(i, j) * vector[j];
        }
    }

    return rs;
}

double *Matrix::operator *(const Matrix &m)
{
    int size =ROWS * COLS;
    double* rs = new double[size];
    fill(rs, rs + size, 0);
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            auto index = i * COLS + j;

            for (int k = 0; k < COLS; k++) {
                rs[index] += value(i, k) * m.value(k, j);
            }
        }
    }

    return rs;
}

Vec1D Matrix::multiply(const Vec1D &vector)
{

    if (COLS != vector.size())
        NC_THROW_ARG_ERROR("Can't multiply vector different size");
    Vec1D rs(COLS);
    for (int i = 0; i < ROWS; i++) {
        rs[i] = 0;

        for (int j = 0; j < COLS; j++) {
            rs[i] += value(i, j) * vector[j];
        }
    }

    return rs;
}

string Matrix::toString()
{
    ostringstream s;
    s << "Matrix {\n";
    for (int i = 0; i < ROWS; i++) {
        s << "\t";
        for (int j = 0; j < COLS; j++) {
            s << value(i, j) << ", ";
        }

        s << "\n";
    }

    s << "}";

    return s.str();
}

Matrix &Matrix::operator=(const Matrix &m)
{
    if (this != &m)
        memcpy(mData.get(), m.mData.get(), sizeof(double) * SIZE);

    return *this;
}

Matrix &Matrix::operator=(Matrix &&m) noexcept
{
    if (this != &m)
        mData = m.mData;

    return *this;
}

void Matrix::revert(Matrix &out)
{
    auto det = determine();
    *out.at(0, 0) = (value(1, 1) * value(2, 2) - value(1, 2) * value(2, 1)) / det;
    *out.at(0, 1) = -(value(0, 1) * value(2, 2) - value(0, 2) * value(2, 1)) / det;
    *out.at(0, 2) = (value(0, 1) * value(1, 2) - value(0, 2) * value(1, 1)) / det;
    *out.at(1, 0) = -(value(1, 0) * value(2, 2) - value(1, 2) * value(2, 0)) / det;
    *out.at(1, 1) = (value(0, 0) * value(2, 2) - value(0, 2) * value(2, 0)) / det;
    *out.at(1, 2) = -(value(0, 0) * value(1, 2) - value(0, 2) * value(1, 0)) / det;
    *out.at(2, 0) = (value(1, 0) * value(2, 1) - value(1, 1) * value(2, 0)) / det;
    *out.at(2, 1) = -(value(0, 0) * value(2, 1) - value(0, 1) * value(2, 0)) / det;
    *out.at(2, 2) = (value(0, 0) * value(1, 1) - value(0, 1) * value(1, 0)) / det;
}

void nucare::math::TransposeMatrix(double *in, double *out, int nRow, int mCol)
{
    for (int i = 0; i < mCol; i++)
    {
        for (int j = 0; j < nRow; j++)
        {
            MAT_2D_VAL(out, nRow, i, j) = MAT_2D_VAL(in, mCol, j, i);
        }
    }
}

void nucare::math::Multi2Matrix(const double *A, const int nRowA, const int mColA,
                  const double *B, const int , const int mColB,
                  double *Res)

{
    for (int i = 0; i < nRowA; i++)
    {
        for (int j = 0; j < mColB; j++)
        {
            MAT_2D_VAL(Res, mColB, i, j) = 0;

            for (int k = 0; k < mColA; k++)
            {
                MAT_2D_VAL(Res, mColB, i, j) = MAT_2D_VAL(Res, mColB, i, j)
                        + MAT_2D_VAL(A, mColA, i, k) * MAT_2D_VAL(B, mColB, k, j);
            }

        }

    }
}



void nucare::math::InverseMatrix(double* out, const double* A, const int n) {
    double a[n][2 * n];

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            a[i][j] = MAT_2D_VAL(A, n, i, j);
        }
    }

    for (int i = 0; i < n; i++) {
        for (int j = n; j < 2 * n; j++) {
            if (i == j % n)
                a[i][j] = 1;
            else
                a[i][j] = 0;
        }
    }
    a[0][n] = 1;

    int temp;
    for (int j = 0; j < n; j++) {
        temp = j;

        for (int i = j + 1; i < n; i++) {
            if (a[i][j] > a[temp][j])
                temp = i;
        }

        if (temp != j) {
            for (int k = 0; k < 2 * n; k++) {
                double temporary = a[j][k];
                a[j][k] = a[temp][k];
                a[temp][k] = temporary;
            }
        }

        for (int i = 0; i < n; i++) {
            if (i != j) {
                double r = a[i][j];

                for (int k = 0; k < 2 * n; k++) {
                    a[i][k] = a[i][k] - (a[j][k] / a[j][j]) * r;
                }

            } else {
                double r = a[i][j];
                for (int k = 0; k < 2 * n; k++) {
                    a[i][k] = a[i][k] / r;
                }
            }
        }

    }

    for (int i = 0; i < n; i++) {
        for (int j = n; j < 2 * n; j++) {
            MAT_2D_VAL(out, n, i, j - n) = a[i][j];
        }
    }
}

void nucare::math::InverseMatrix(double** out, const double** A, const int n)
{
    double a[n][2 * n];

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            a[i][j] = A[i][j];
        }
    }

    for (int i = 0; i < n; i++) {
        for (int j = n; j < 2 * n; j++) {
            if (i == j % n)
                a[i][j] = 1;
            else
                a[i][j] = 0;
        }
    }
    a[0][n] = 1;

    int temp;
    for (int j = 0; j < n; j++) {
        temp = j;

        for (int i = j + 1; i < n; i++) {
            if (a[i][j] > a[temp][j])
                temp = i;
        }

        if (temp != j) {
            for (int k = 0; k < 2 * n; k++) {
                double temporary = a[j][k];
                a[j][k] = a[temp][k];
                a[temp][k] = temporary;
            }
        }

        for (int i = 0; i < n; i++) {
            if (i != j) {
                double r = a[i][j];

                for (int k = 0; k < 2 * n; k++) {
                    a[i][k] = a[i][k] - (a[j][k] / a[j][j]) * r;
                }

            } else {
                double r = a[i][j];
                for (int k = 0; k < 2 * n; k++) {
                    a[i][k] = a[i][k] / r;
                }
            }
        }

    }

    for (int i = 0; i < n; i++) {
        for (int j = n; j < 2 * n; j++) {
            out[i][j - n] = a[i][j];
        }
    }
}

void nucare::math::InverseMatrix(Vec2D& out, const Vec2D& A, const int n)
{
    double a[n][2 * n];

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            a[i][j] = A[i][j];
        }
    }

    for (int i = 0; i < n; i++) {
        for (int j = n; j < 2 * n; j++) {
            if (i == j % n)
                a[i][j] = 1;
            else
                a[i][j] = 0;
        }
    }
    a[0][n] = 1;

    int temp;
    for (int j = 0; j < n; j++) {
        temp = j;

        for (int i = j + 1; i < n; i++) {
            if (a[i][j] > a[temp][j])
                temp = i;
        }

        if (temp != j) {
            for (int k = 0; k < 2 * n; k++) {
                double temporary = a[j][k];
                a[j][k] = a[temp][k];
                a[temp][k] = temporary;
            }
        }

        for (int i = 0; i < n; i++) {
            if (i != j) {
                double r = a[i][j];

                for (int k = 0; k < 2 * n; k++) {
                    a[i][k] = a[i][k] - (a[j][k] / a[j][j]) * r;
                }

            } else {
                double r = a[i][j];
                for (int k = 0; k < 2 * n; k++) {
                    a[i][k] = a[i][k] / r;
                }
            }
        }

    }

    for (int i = 0; i < n; i++) {
        for (int j = n; j < 2 * n; j++) {
            out[i][j - n] = a[i][j];
        }
    }
}

void nucare::math::MultiMatrix2Dby1D(const double* A, const int nRowA, const int mColA,
                                             const double* B, const int , double* Res)
{

    for (int i = 0; i < nRowA; i++)
    {
        Res[i] = 0;

        for (int k = 0; k < mColA; k++)
        {
            Res[i] = Res[i] + MAT_2D_VAL(A, mColA, i, k) * B[k];
        }
    }
}

void nucare::math::MultiMatrix2Dby1D(const Vec2D& A, const Vec1D& B, Vec1D& Res) {

    if (A.empty() || A[0].size() != B.size() || A[0].size() != Res.size()) {
        NC_THROW_ARG_ERROR("Invalid size of matrix");
    }

    for (nucare::suint i = 0; i < A.size(); i++)
    {
        Res[i] = 0;

        for (nucare::suint k = 0; k < A[0].size(); k++)
        {
            Res[i] = Res[i] + A[i][k] * B[k];
        }
    }
}

void nucare::math::AddMatrix1D(const double* A, const double* B, const int N, double* Res)
{
    for (int i = 0; i < N; i++)
    {
        Res[i] =  A[i] + B[i];
    }
}

double nucare::math::Rsquare_Fit(const double* y, const double* f0, const int n) // f0: fit data
{
    double sumf0 = 0;
    double sumYminusf0 = 0;

    for (int i = 0; i < n; i++)
    {
        sumf0 = sumf0 + f0[i];
        sumYminusf0 = sumYminusf0 + (y[i] - f0[i])*(y[i] - f0[i]);
    }

    double meanf0 = sumf0 / (double)n;

    double MSE = sumYminusf0 /(double)n;

    double SSE = MSE*(double)n;

    double TSS = 0;
    for (int i = 0; i < n; i++)
    {
        TSS = TSS + (y[i] - meanf0)*(y[i] - meanf0);
    }


    double R2fit = (double)1.0 - SSE / TSS;

    return R2fit;

}
