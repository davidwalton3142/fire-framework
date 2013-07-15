#include "SHMat.hpp"

SHMat::SHMat(const glm::mat4& rotation, int nBands)
{
	glm::mat3 subMat
		(
		rotation[0][0], rotation[0][1], rotation[0][2],
		rotation[1][0], rotation[1][1], rotation[1][2],
		rotation[2][0], rotation[2][1], rotation[2][2]
		);
	init(subMat, nBands);
}

std::vector<float> SHMat::operator * (const std::vector<float>& p)
{
	if(p.size() != ((blocks.size() + 1) * (blocks.size() + 1))) 
		throw new MatDimException;

	std::vector<float> ans;

	for(int i = 0; i < blocks.size(); ++i)
	{
		std::vector<float> subVec(p.begin() + (i*i), p.begin() + (i+1)*(i+1));
		std::vector<float> subProd = blocks[i] * subVec;

		for(std::vector<float>::iterator i = subProd.begin(); i != subProd.end(); ++i)
			ans.push_back(*i);
	}

	return ans;
}

void SHMat::init(const glm::mat3& rotation, int nBands)
{
	Matrix<float> R_o(rotation);
	/* Rearrange coeffts of R */
	Matrix<float> R(3,3);
	R(0,0) = R_o(1,1);
	R(0,1) = R_o(2,1);
	R(0,2) = R_o(1,0);
	R(1,0) = R_o(2,1);
	R(1,1) = R_o(2,2);
	R(1,2) = R_o(2,0);
	R(2,0) = R_o(0,1);
	R(2,1) = R_o(0,2);
	R(2,2) = R_o(0,0);

	for(int l = 0; l < nBands; ++l)
	{
		Matrix<float> mat(2*l + 1, 2*l + 1);
		for(int m = -l; m <= l; ++m)
			for(int n = -l; n <= l; ++n)
			{
				mat.i(m,n) = M(l,m,n,R);
			}
		blocks.push_back(mat);
	}
}

float M(int l, int m, int n, const Matrix<float>& R)
{
	return u(l,m,n) * U(l,m,n,R)
		 + v(l,m,n) * V(l,m,n,R)
		 + w(l,m,n) * W(l,m,n,R);
}

float SHMat::P(int i, int l, int m, int n, const Matrix<float>& R)
{
	if(n == l)
		return R.i(i,1) * blocks[l-1].i(m,l-1) - R.i(i,-1) * blocks[l-1].i(m,-l+1);
	if(n == -l)
		return R.i(i,1) * blocks[l-1].i(m,-l+1) + R.i(i,-1) * blocks[l-1].i(m,l-1);
	else
		return R.i(i,0) * blocks[l-1].i(m,n);
}

float SHMat::u(int l, int m, int n)
{
	if(n == l || n == -l)
		return sqrt(((l + m) * (l - m)) / ((2*l) * (2*l - 1)));
	else 
		return sqrt(((l + m) * (l - m)) / ((l + n) * (l - n)));
}

float SHMat::v(int l, int m, int n)
{
	float num = (1 + del(m,0)) * (l + abs(m) - 1) * (l + abs(m));

	if(n == l || n == -l)
		float den = (2 * l) * (2*l - 1);
	else
		float den = (l + n) * (l - n);

	return 0.5 * sqrt(num / den) * (1 - 2*del(m,0));
}

float SHMat::w(int l, int m, int n)
{
	float num = (l - abs(m) - 1) * (l - abs(m));

	if(n == l || n == -l)
		float den = (2 * l) * (2*l - 1);
	else
		float den = (l + n) * (l - n);

	return -0.5 * sqrt(num / den) * (1 - del(m,0));
}

float SHMat::U(int l, int m, int n, const Matrix<float>& R)
{
	return P(0, l, m, n, R);
}

float SHMat::V(int l, int m, int n, const Matrix<float>& R)
{
	if(m == 0)
		return P(1, l, 1, n) * P(-1, l, -1, n);
	else if(m > 0)
		return P(1, l, m-1, n) * sqrt(1 + del(m,1)) - P(-1, l, -m+1, n) * (1 - del(m,1));
	else //m < 0
		return P(1, l, m+1, n) * (1 + del(m,-1)) + P(-1, 1, -m-1, n) * sqrt(1 - del(m,-1)); 
}

float SHMat::W(int l, int m, int n, const Matrix<float>& R)
{
	/* Shouldn't be called with m == 0 */
	if(m == 0)
		throw new MatDimException; 
	else if(m > 0)
		return P(1, l, m+1, n) + P(-1, l, -m-1, n);
	else //m < 0
		return P(1, l, m-1, n) - P(-1, l, -m+1, n);
}
