#pragma once
#include "vec.h"
template <typename T, int rows, int cols> struct Mat;
typedef Mat<float, 4, 4> Mat4;

template <typename T, int rows, int cols>
struct Mat
{
	T data[rows][cols];

	Mat()
	{
		for (int i = 0; i < rows; ++i)
		{
			for (int j = 0; j < cols; ++j)
			{
				data[i][j] = 0;
			}
		}
		for (int i = 0; i < rows; ++i)
		{
			data[i][i] = 1;
		}
	}

	Mat(const T& t)
	{
		for (int i = 0; i < rows; ++i)
		{
			for (int j = 0; j < cols; ++j)
			{
				data[i][j] = t;
			}
		}
	}

	T& operator ()(const int row, const int col)
	{
		return data[row][col];
	}

	const T& operator ()(const int row, const int col) const
	{
		return data[row][col];
	}

	Mat& operator +=(const Mat& m)
	{
		for (int i = 0; i < rows; ++i)
		{
			for (int j = 0; j < cols; ++j)
			{
				data[i][j] += m.data[i][j];
			}
		}
		return *this;
	}

	Mat& operator -=(const Mat& m)
	{
		for (int i = 0; i < rows; ++i)
		{
			for (int j = 0; j < cols; ++j)
			{
				data[i][j] -= m.data[i][j];
			}
		}
		return *this;
	}

	Mat& operator *=(const T& t)
	{
		for (int i = 0; i < rows; ++i)
		{
			for (int j = 0; j < cols; ++j)
			{
				data[i][j] *= t;
			}
		}
		return *this;
	}

    Mat4 operator * (const Mat4& m) const
    {
        Mat4 r(0.0f);
        for(int i = 0; i < 4; i++)
        {
            for(int j = 0; j < 4; j++)
            {
                for(int k = 0; k < 4; k++)
                {
                    r(i, k) += (*this)(i, j) * m(j, k);
                }
            }
        }
        return r;
    }
    
	Mat& operator /=(const T& t)
	{
		const T inv = 1 / t;
		for (int i = 0; i < rows; ++i)
		{
			for (int j = 0; j < cols; ++j)
			{
				data[i][j] *= inv;
			}
		}
		return *this;
	}

	Mat operator +(const Mat& m)
	{
		return Mat(*this) += m;
	}

	Mat operator -(const Mat& m)
	{
		return Mat(*this) -= m;
	}

	Mat operator *(const T& t)
	{
		return Mat(*this) *= t;
	}

	Mat operator /(const T& t)
	{
		return Mat(*this) /= t;
	}

	Vec<T, cols> operator *(const Vec<T, cols>& v)
	{
		Vec<T, cols> r;
		for (int i = 0; i < rows; ++i)
		{
			for(int j = 0; j < cols; ++j)
			{
				r[i] += data[i][j] * v[j];
			}
		}
		return r;
	}
    

	static Mat4 makeXRotation(const float angle)
	{
		Mat4 r;
		float theta = angle / 180.0f * (float)PI;
		float sin_theta = sinf(theta);
		float cos_theta = cosf(theta);
		r(1, 1) = cos_theta;
		r(2, 1) = sin_theta;
		r(1, 2) = -sin_theta;
		r(2, 2) = cos_theta;
		return r;
	}

	static Mat4 makeYRotation(const float angle)
	{
		Mat4 r;
		float theta = angle / 180.0f * (float)PI;
		float sin_theta = sinf(theta);
		float cos_theta = cosf(theta);
		r(0, 0) = cos_theta;
		r(2, 0) = sin_theta;
		r(0, 2) = -sin_theta;
		r(2, 2) = cos_theta;
		return r;
	}

	static Mat4 makeZRotation(const float angle)
	{
		Mat4 r;
		float theta = angle / 180.0f * (float)PI;
		float sin_theta = sinf(theta);
		float cos_theta = cosf(theta);
		r(0, 0) = cos_theta;
		r(1, 0) = sin_theta;
		r(0, 1) = -sin_theta;
		r(1, 1) = cos_theta;
		return r;
	}

	static Mat4 makeScale(const Vec3& scale)
	{
		Mat4 r;
		r(0, 0) = scale[0];
		r(1, 1) = scale[1];
		r(2, 2) = scale[2];
		return r;
	}

	static Mat4 makeTranslation(const Vec3& t)
	{
		Mat4 r;
		for (int i = 0; i < 3; ++i)
		{
			r(i, 3) = t[i];
		}
		return r;
	}

	static Mat4 makePerspective(const float fovy, const float aspect_ratio, const float z_near,
		const float z_far)
	{
		Mat4 r;

		return r;
	}
};


