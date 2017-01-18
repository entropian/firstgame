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
        float ang = fovy*0.5f*(float)PI/180.0f;
        float f = fabs(sin(ang)) < EPS ? 0 : 1/tan(ang);
        if(abs(aspect_ratio) > EPS)
        {
            r(0, 0) = f/aspect_ratio;
        }

        r(1, 1) = f;

        if(fabs(z_far - z_near) > EPS)
        {
            // TODO: figure out why the matrix4.h version is positive
            //       and figure out how the glm version works
            r(2, 2) = -(z_far + z_near)/(z_far - z_near);
            r(2, 3) = (float)(-2.0*z_far*z_near/(z_far-z_near));
        }

        r(3, 2) = -1.0;
        return r;
	}
};

static Mat4 lookAt(const Vec3& pos, const Vec3& look_point, const Vec3& up_vec)
{
    // TODO: test this
    Vec3 z_vec(normalize(pos - look_point));
    Vec3 x_vec(normalize(cross(z_vec, up_vec)));
    Vec3 y_vec(cross(x_vec, z_vec));
    Mat4 ret(Mat4::makeTranslation(-pos));
    ret(0, 0) = x_vec[0];
    ret(1, 0) = x_vec[1];
    ret(2, 0) = x_vec[2];
    ret(0, 1) = y_vec[0];
    ret(1, 1) = y_vec[1];
    ret(2, 1) = y_vec[2];
    ret(0, 2) = z_vec[0];
    ret(1, 2) = z_vec[1];
    ret(2, 2) = z_vec[2];
    return ret;
}


