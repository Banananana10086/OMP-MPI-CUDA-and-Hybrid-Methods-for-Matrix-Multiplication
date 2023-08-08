#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
using namespace std;

#include "Matrix.h"

//MPI_Send��MPI_Recvʵ��
int main(int argc, char** argv) {
	//����ߴ�
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
		for (i = 1; i < numprocs; ++i) {
			MPI_Send(B.elements, B.height * B.width, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
		}

		// ������A�ĸ��з��͸������ӽ���
		for (i = 1; i < numprocs; ++i) {
			MPI_Send(A.elements + (i - 1) * line * nnn, line * nnn, MPI_FLOAT, i, 1, MPI_COMM_WORLD);
		}

		// ���ܴӽ��̵ļ�����
		for (k = 1; k < numprocs; ++k) {
			MPI_Recv(ans.elements, line * kkk, MPI_FLOAT, k, 3, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
			//������������C
			for (i = 0; i < line; ++i) {
				for (j = 0; j < kkk; ++j) {
					C.elements[((k - 1) * line + i) * kkk + j] = ans.elements[i * kkk + j];
				}
			}
		}

		//����Aʣ�µ�����
		float temp;
		for (i = (numprocs - 1) * line; i < mmm; ++i) {
			for (j = 0; j < kkk; ++j) {
				temp = 0.0f;
				for (k = 0; k < nnn; ++k) {
					temp += A.elements[i * nnn + k] * B.elements[k * kkk + j];
				}
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
		cout << "MPI method v1 runtime: \t" << time_MPI << "s\t" <<
			"speedUP:" << time_single / time_MPI << "\t  " <<
			"maximum error: " << maxErrorOfMatrixs(C_single, C) << "\t" <<
			"Efficiency: " << time_single / time_MPI / numprocs << endl;
		/*printMatrix(A);
		printMatrix(B);
		printMatrix(C);*/
	}
	// �ӽ��̽������ݣ������������͸�������
	else {
		MPI_Recv(B.elements, B.width * B.height, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(buffer.elements, line * nnn, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		// ������
		for (i = 0; i < line; ++i) {
			for (j = 0; j < kkk; ++j) {
				float temp = 0.0f;
				for (k = 0; k < nnn; ++k) {
					temp += buffer.elements[i * nnn + k] * B.elements[k * kkk + j];
				}
				ans.elements[i * kkk + j] = temp;
			}
		}
		// ��������͸�������
		MPI_Send(ans.elements, line * kkk, MPI_INT, 0, 3, MPI_COMM_WORLD);
	}

	MPI_Finalize();//����
	return 0;
}