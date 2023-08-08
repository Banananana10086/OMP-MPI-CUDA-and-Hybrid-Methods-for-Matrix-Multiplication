#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
using namespace std;

#include "Matrix.h"

/// <summary>
/// ��ͨ����˷�
/// </summary>
void matrixMUL(float* A, float* B, float* C, int m, int n, int k) {
	float temp;
	for (int i = 0; i < m; ++i)
	{
		for (int j = 0; j < k; ++j)
		{
			temp = 0.0f;
			for (int l = 0; l < n; ++l)
			{
				temp += A[i * n + l] * B[l * k + j];
			}
			C[i * k + j] = temp;
		}
	}
}

//MPI_Scatter��MPI_Gatherʵ��
int main(int argc, char** argv) {
	int mmm, nnn, kkk;
	mmm = 1024;
	nnn = 1024;
	kkk = 1024;
	if (argc >= 4) {
		mmm = atoi(argv[1]);
		nnn = atoi(argv[2]);
		kkk = atoi(argv[3]);
	}
	// MPI��ʼ��
	int rank = 0, numprocs = 0;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);  //��ǰ���̺�
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);  //���̸���
	MPI_Status status;

	// ��ʼ������
	Matrix A, B, C, buffer, ans;
	matrixCreate(A, mmm, nnn);
	matrixCreate(B, nnn, kkk);
	matrixCreate(C, mmm, kkk);

	int line = mmm / numprocs;
	// �����С���ڵ���Ҫ��������ݴ�С������ʱֻ���עʵ�������ǲ���
	matrixCreate(buffer, line, nnn);
	matrixCreate(ans, line, kkk);

	int i, j, k;
	// ��������
	if (rank == 0) {
		printf("order of matrix: %d-%d-%d\n", mmm, nnn, kkk);
		// ������A��B�����ֵ
		initMatrix(A);
		initMatrix(B);

		double start = MPI_Wtime(), stop, time_MPI;
		// ������B���͸���������
		//���ݹ㲥
		MPI_Bcast(B.elements, B.height * B.width, MPI_FLOAT, 0, MPI_COMM_WORLD);

		// ������A�ĸ��з��͸������ӽ���
		//���ݷַ�
		MPI_Scatter(A.elements, line * A.width, MPI_FLOAT, buffer.elements, line * nnn, MPI_FLOAT, 0, MPI_COMM_WORLD);

		//���㱾�ؽ��
		matrixMUL(buffer.elements, B.elements, ans.elements, buffer.height, buffer.width, B.width);

		//����ۼ�
		MPI_Gather(ans.elements, line * kkk, MPI_FLOAT, C.elements, line * kkk, MPI_FLOAT, 0, MPI_COMM_WORLD);

		//ʣ���д��������������������
		float temp;
		int rest = mmm % numprocs;
		if (rest != 0) {
			cout << "����ʣ���д���ʣ���У�" << rest << endl;
			for (i = mmm - rest; i < mmm; i++)
				for (j = 0; j < kkk; j++) {
					temp = 0.0f;
					for (k = 0; k < nnn; k++)
						temp += A.elements[i * nnn + k] * B.elements[k * kkk + j];
					C.elements[i * kkk + j] = temp;
				}
		}

		stop = MPI_Wtime();
		time_MPI = stop - start;


		// ���ݵ��̼߳�����Ϊ��׼��֤MPI�������Ƿ���ȷ
		double time_single;
		Matrix C_single;
		matrixCreate(C_single, mmm, kkk);
		start = MPI_Wtime();
		matrixMulSingle(A, B, C_single);
		stop = MPI_Wtime();
		time_single = stop - start;

		cout << "single method runtime: \t " << time_single << "s" << endl;
		cout << "MPI method v2 runtime: \t" << time_MPI << "s\t" <<
			"speedUP:" << time_single / time_MPI << "\t  " <<
			"maximum error: " << maxErrorOfMatrixs(C_single, C) << "\t" <<
			"Efficiency: " << time_single / time_MPI / numprocs << endl;
		//printMatrix(C_single);
		//printMatrix(C);
	}
	// �ӽ��̽������ݣ������������͸�������
	else {
		MPI_Bcast(B.elements, B.width * B.height, MPI_FLOAT, 0, MPI_COMM_WORLD);
		MPI_Scatter(A.elements, line * nnn, MPI_FLOAT, buffer.elements, line * nnn, MPI_FLOAT, 0, MPI_COMM_WORLD);
		// ���㱾��
		matrixMUL(buffer.elements, B.elements, ans.elements, buffer.height, buffer.width, B.width);
		// ����ۼ�
		MPI_Gather(ans.elements, line * kkk, MPI_FLOAT, C.elements, line * kkk, MPI_FLOAT, 0, MPI_COMM_WORLD);

	}

	MPI_Finalize();//����
	return 0;
}