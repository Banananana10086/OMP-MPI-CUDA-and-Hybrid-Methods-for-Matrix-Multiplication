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

void matrixCreate(Matrix& M, int m, int n);
void initMatrix(Matrix& M);
void matrixCreateAndInit(Matrix& M, int m, int n);
void printMatrix(Matrix& M);
bool matrixCompare(Matrix& A, Matrix& B, float er);
float maxErrorOfMatrixs(Matrix A, Matrix B);
void matrixMulSingle(Matrix& A, Matrix& B, Matrix& C);
