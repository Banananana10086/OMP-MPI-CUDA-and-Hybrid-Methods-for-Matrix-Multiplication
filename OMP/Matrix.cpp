#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <omp.h>
#include <stdio.h>
#include <time.h>
#include <thread>
#include <math.h>
using namespace std;

#include "Matrix.h"

/// <summary>
/// ��������,�������Ԫ�ص��ڴ�ռ�
/// </summary>
/// <param name="M">Matrix M</param>
/// <param name="m">��������</param>
/// <param name="n">��������</param>
void matrixCreate(Matrix& M, int m, int n) {
	M.height = m;
	M.width = n;
	M.stride = M.width;
	M.elements = new float[M.height * M.width];
}
/// <summary>
/// Ϊ�����е�Ԫ�������ֵ
/// </summary>
/// <param name="M">Matrix M</param>
void initMatrix(Matrix& M) {
	int m = M.height;
	int n = M.width;
	srand((unsigned int)time(NULL));
	for (int i = 0; i < m; ++i) {
		for (int j = 0; j < n; ++j) {
			M.elements[i * n + j] = float((rand() % 10000) - (rand() % 10000)) / 100.0;  //rand()����0-�������������
		}
	}
}
/// <summary>
/// ��������ʼ�����󣺷����ڴ�ռ䲢��ÿ��Ԫ�������ֵ
/// </summary>
/// <param name="M">Matrix M</param>
/// <param name="m">��������</param>
/// <param name="n">��������</param>
void matrixCreateAndInit(Matrix& M, int m, int n) {
	matrixCreate(M, m, n);
	initMatrix(M);
}
/// <summary>
/// ��ӡ����Ԫ��
/// </summary>
/// <param name="M">Matrix M</param>
void printMatrix(Matrix& M) {
	int m = M.height;
	int n = M.width;
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			printf("%12.3f\t", M.elements[i * n + j]);
		}
		cout << endl;
	}
	cout << endl;
}
/// <summary>
/// ����OMP���м��㣺�Ƚ���������Ԫ���Ƿ����
/// </summary>
/// <param name="A">Matrix A</param>
/// <param name="B">Matrix B</param>
/// <param name="er">�����������</param>
/// <returns>��������ȷ���True������ȷ���False</returns>
bool matrixCompare(Matrix& A, Matrix& B, float er) {
	if (A.height != B.height || A.width != B.width)
		return false;

	int m = A.height;
	int n = A.width;
	const int num_threads = (int)std::thread::hardware_concurrency();
	int* res = new int[num_threads];//���ڱ�����
	for (int i = 0; i < num_threads; i++)
		res[i] = 1;
	omp_set_num_threads(num_threads);
#pragma omp parallel
	{
		int id = omp_get_thread_num();
		for (int i = id; i < m * n; i += num_threads) {
			if (abs(A.elements[i] - B.elements[i]) > er) {
				res[id] = 0;
				break;
			}
		}
	}
	//�������
	for (int i = 0; i < num_threads; i++) {
		if (res[i] == 0)
			return false;
	}
	return true;
}
/// <summary>
/// �������������
/// </summary>
/// <param name="A">Matrix A</param>
/// <param name="B">Matrix B</param>
/// <returns>������</returns>
/// <returns>������</returns>
float maxErrorOfMatrixs(Matrix A, Matrix B) {

	float maxError = 0.0f;
	int length = A.height * A.width;
	float temp;
	for (int i = 0; i < length; i++) {
		temp = abs(A.elements[i] - B.elements[i]);
		if (temp > maxError) {
			maxError = temp;
		}
	}
	return maxError;
}
/// </summary>
/// <param name="A">Matrix A</param>
/// <param name="B">Matrix B</param>
/// <param name="C">Matrix C</param>
void matrixMulSingle(Matrix& A, Matrix& B, Matrix& C) {
	int a = A.height;
	int b = A.width;
	int c = B.width;
	float temp;
	for (int i = 0; i < a; i++)
	{
		for (int j = 0; j < c; j++)
		{
			temp = 0.0f;
			for (int k = 0; k < b; k++)
			{
				temp += A.elements[i * b + k] * B.elements[k * c + j];
			}
			C.elements[i * c + j] = temp;
		}
	}
}