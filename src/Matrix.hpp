#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <exception>
#include <vector>
#include <glm.hpp>

class MatDimException : public std::exception {};

/* Matrix
 * Fairly bare-bones arbitrary size matrix class.
 * Used by SHMat.
 * N.B. : This matrix class follows the usual row-major convention,
 * differing from the column-major glm::mat* classes.
 */
template <typename T>
class Matrix
{
public:
	Matrix(unsigned rows, unsigned cols) : r(rows), c(cols) {zeroData();}
	Matrix(unsigned n) : r(n), c(n) {zeroData();}
	Matrix(unsigned n, T v);

	Matrix(unsigned rows, unsigned cols, std::vector<T> vals);

	Matrix(const glm::mat4& m);
	Matrix(const glm::mat3& m);

	Matrix(const Matrix& other);

	~Matrix();

	const T& operator () (unsigned i, unsigned j) const;
	T& operator () (unsigned i, unsigned j);

	Matrix<T>& operator += (const Matrix<T>& m);
	Matrix<T> operator + (const Matrix<T>& m);
	Matrix<T> operator * (const Matrix<T>& m);
	std::vector<T> operator * (const std::vector<T>& v);

	const unsigned r, c;
private:
	T** data;

	void allocData();
	void zeroData();
};

template <typename T>
Matrix<T> operator * (const T& s, const Matrix<T>& m);
template <typename T>
Matrix<T> operator * (const Matrix<T>& m, const T& s);
template <typename T>
std::vector<T> operator * (const std::vector<T>& v, const Matrix<T>& m);

template <typename T>
Matrix<T>::Matrix(unsigned n, T v)
	:r(n), c(n)
{
	allocData();
	for(unsigned i = 0; i < r; ++i)
		for(unsigned j = 0; j < c; ++j)
			data[i][j] = i == j ? v : 0;
}

template <typename T>
Matrix<T>::Matrix(unsigned rows, unsigned cols, std::vector<T> vals)
	:r(rows), c(cols)
{
	if(rows * cols != vals.size()) throw new MatDimException;
	allocData();
	for(unsigned i = 0; i < r; ++i)
		for(unsigned j = 0; j < c; ++j)
			data[i][j] = vals[(i*c) + j];
}

template <typename T>
Matrix<T>::Matrix(const glm::mat4& m)
	:r(4), c(4)
{
	allocData();
	for(unsigned i = 0; i < r; ++i)
		for(unsigned j = 0; j < c; ++j)
			data[i][j] = m[j][i];
}

template <typename T>
Matrix<T>::Matrix(const glm::mat3& m)
	:r(3), c(3)
{
	allocData();
	for(unsigned i = 0; i < r; ++i)
		for(unsigned j = 0; j < c; ++j)
			data[i][j] = m[j][i];
}

template <typename T>
Matrix<T>::Matrix(const Matrix& other)
	:r(other.r), c(other.c)
{
	allocData();
	for(unsigned i = 0; i < r; ++i)
		for(unsigned j = 0; j < c; ++j)
			data[i][j] = other(i, j);
}

template <typename T>
Matrix<T>::~Matrix()
{
	for(unsigned i = 0; i < r; ++i)
		delete [] data[i];
	delete data;
}

template <typename T>
const T& Matrix<T>::operator () (unsigned i, unsigned j) const
{
	if(i >= r || j >= c) throw new MatDimException;
	return data[i][j];
}

template <typename T>
T& Matrix<T>::operator () (unsigned i, unsigned j)
{
	if(i >= r || j >= c) throw new MatDimException;
	return data[i][j];
}

template <typename T>
Matrix<T>& Matrix<T>::operator += (const Matrix& m)
{
	if(r != m.r || c != m.c) throw new MatDimException;
	for(unsigned i = 0; i < r; ++i)
		for(unsigned j = 0; j < c; ++j)
			data[i][j] += m.data[i][j];
	return *this;
}

template <typename T>
Matrix<T> Matrix<T>::operator + (const Matrix& m)
{
	Matrix temp(*this);
	return (temp += m);
}

template <typename T>
Matrix<T> Matrix<T>::operator * (const Matrix<T>& m)
{
	if(c != m.r) throw new MatDimException;
	Matrix ans(r, m.c);
	for(unsigned i = 0; i < ans.r; ++i)
		for(unsigned j = 0; j < ans.c; ++j)
			for(unsigned k = 0; k < c; ++k)
				ans.data[i][j] += data[i][k] * m.data[k][j];
	return ans;
}

template <typename T>
std::vector<T> Matrix<T>::operator * (const std::vector<T>& v)
{
	if(v.size() != c) throw new MatDimException;
	std::vector<T> ans(r, 0);

	for(unsigned i = 0; i < r; ++i)
		for(unsigned j = 0; j < c; ++j)
			ans[j] += data[i][j] * v[j];

	return ans;
}

template <typename T>
std::vector<T> operator * (const std::vector<T>& v, const Matrix<T>& m)
{
	if(v.size() != r) throw new MatDimException;
	std::vector<T> ans(c, 0);

	for(unsigned j = 0; j < c; ++j)
		for(unsigned i = 0; i < r; ++i)
			ans[i] += data[i][j] * v[i];

	return ans;
}

template <typename T>
Matrix<T> operator * (const T& s, const Matrix<T>& m)
{
	Matrix<T> ans(m.r, m.c);
	for(unsigned i = 0; i < m.r; ++i)
		for(unsigned j = 0; j < m.c; ++j)
			ans(i,j) = s * m(i,j);
	return ans;
}

template <typename T>
Matrix<T> operator * (const Matrix<T>& m, const T& s)
{
	Matrix<T> ans(m.r, m.c);
	for(unsigned i = 0; i < m.r; ++i)
		for(unsigned j = 0; j < m.c; ++j)
			ans(i,j) = m(i,j) * s;
	return ans;
}

template <typename T>
void Matrix<T>::allocData()
{
	data = new T*[r];
	for(unsigned i = 0; i < r; ++i)
		data[i] = new T[c];
}

template <typename T>
void Matrix<T>::zeroData()
{
	allocData();
	for(unsigned i = 0; i < r; ++i)
		for(unsigned j = 0; j < c; ++j)
			data[i][j] = 0;
}

#endif
