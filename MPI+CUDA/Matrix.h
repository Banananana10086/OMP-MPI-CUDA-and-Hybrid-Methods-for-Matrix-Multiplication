#include <stdio.h>
#include <iostream>
using namespace std;

typedef struct {
	int height;  //��������
	int width;  //��������
	//��λ����M��N��Ԫ�ط�ʽ��M.elements[M * M.stride + N]
	//��Ԫ��һ����������������
	//������ĳ������Ӿ���֮����ڶ�λ�Ӿ����Ԫ��
	int stride;
	float* elements;  //��һά�����ʾ�����е�Ԫ��
} Matrix;
/// <summary>
/// ��������,�������Ԫ�ص��ڴ�ռ�
/// </summary>
/// <param name="M">Matrix M</param>
/// <param name="m">��������</param>
/// <param name="n">��������</param>
void matrixCreate(Matrix& M, int m, int n);
/// <summary>
/// Ϊ�����е�Ԫ�������ֵ
/// </summary>
/// <param name="M">Matrix M</param>
void initMatrix(Matrix& M);
/// <summary>
/// ��������ʼ�����󣺷����ڴ�ռ䲢��ÿ��Ԫ�������ֵ
/// </summary>
/// <param name="M">Matrix M</param>
/// <param name="m">��������</param>
/// <param name="n">��������</param>
void matrixCreateAndInit(Matrix& M, int m, int n);
/// <summary>
/// ��ӡ����Ԫ��
/// </summary>
/// <param name="M">Matrix M</param>
void printMatrix(Matrix& M);
///// <summary>
///// �Ƚ���������Ԫ���Ƿ����
///// </summary>
///// <param name="A">Matrix A</param>
///// <param name="B">Matrix B</param>
///// <param name="er">�����������</param>
///// <returns>��������ȷ���True������ȷ���False</returns>
//bool matrixCompare(Matrix& A, Matrix& B, float er);
/// <summary>
/// �������������
/// </summary>
/// <param name="A">Matrix A</param>
/// <param name="B">Matrix B</param>
/// <returns>������</returns>
float maxErrorOfMatrixs(Matrix A, Matrix B);
/// <summary>
/// ���߳���ͨ����˷�
/// </summary>
/// <param name="A"></param>
/// <param name="B"></param>
/// <param name="C"></param>
void matrixMulSingle(Matrix& A, Matrix& B, Matrix& C);
