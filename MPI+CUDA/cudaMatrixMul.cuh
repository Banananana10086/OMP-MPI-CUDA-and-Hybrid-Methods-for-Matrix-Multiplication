#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include "Matrix.h"
#define BLOCK_SIZE 16

int calculateDimGrid(int a, int b) {
	float A = float(a);
	float B = float(b);
	float C = (A + (B - 1)) / B;
	return int(C);
}

/// <summary>
/// ������ģ�ֱ�Ӽ���汾
/// </summary>
__global__ void MatMulKernelV1(Matrix A, Matrix B, Matrix C)
{
	int m = A.height;
	int n = A.width;
	int k = B.width;
	int row = blockIdx.y * blockDim.y + threadIdx.y;
	int col = blockIdx.x * blockDim.x + threadIdx.x;
	float sum = 0;
	if (col < k && row < m)
	{
		for (int i = 0; i < n; i++)
		{
			sum += A.elements[row * n + i] * B.elements[i * k + col];
		}
		C.elements[row * k + col] = sum;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// CUDA����˷�����ͨ����
/// C = A �� B
/// </summary>
/// <param name="A">������A</param>
/// <param name="B">������B</param>
/// <param name="C">������C</param>
/// <param name="times">�������</param>
/// <param name="warmupTimes">Ԥ�ȴ���</param>
/// <returns>�����ں�����ʱ�䣨��λ�����룩</returns>
float matrixMulCudaV1(const Matrix A, const Matrix B, Matrix C, int times, int warmupTimes = 10)
{
	float start, end, diffTime;
	//���ؾ����Դ�
	Matrix d_A;
	d_A.width = A.width; d_A.height = A.height; d_A.stride = A.stride;
	size_t size = A.width * A.height * sizeof(float);
	cudaMalloc(&d_A.elements, size);
	cudaMemcpy(d_A.elements, A.elements, size,
		cudaMemcpyHostToDevice);
	Matrix d_B;
	d_B.width = B.width; d_B.height = B.height; d_B.stride = B.stride;
	size = B.width * B.height * sizeof(float);
	cudaMalloc(&d_B.elements, size);
	cudaMemcpy(d_B.elements, B.elements, size,
		cudaMemcpyHostToDevice);

	Matrix d_C;
	d_C.width = C.width; d_C.height = C.height; d_C.stride = C.stride;
	size = C.width * C.height * sizeof(float);
	cudaMalloc(&d_C.elements, size);

	//������
	cudaThreadSynchronize();
	dim3 dimBlock(BLOCK_SIZE, BLOCK_SIZE);
	//dim3 dimGrid(B.width / dimBlock.x, A.height / dimBlock.y);
	dim3 dimGrid(calculateDimGrid(B.width, dimBlock.x), calculateDimGrid(A.height, dimBlock.y));
	start = clock();
	for (int num = 0; num < warmupTimes; ++num) {
		MatMulKernelV1 << <dimGrid, dimBlock >> > (d_A, d_B, d_C);
		cudaDeviceSynchronize();
	}//Ԥ��������ø��Ӿ�ȷ�ļ�ʱ
	for (int num = 0; num < 100; ++num) {
		MatMulKernelV1 << <dimGrid, dimBlock >> > (d_A, d_B, d_C);
		cudaDeviceSynchronize();//CPU�ȴ�GPU�����������
	}//����N�Σ�Ϊ��þ�ȷ��ʱ
	end = clock();
	//���ؾ���C���ڴ�
	cudaMemcpy(C.elements, d_C.elements, size, cudaMemcpyDeviceToHost);
	diffTime = end - start;
	//printMatrix(C);
	//�ͷſռ�
	cudaFree(d_A.elements);
	cudaFree(d_B.elements);
	cudaFree(d_C.elements);
	return diffTime / float(times);
}

/// <summary>
/// �õ�����Ԫ��
/// </summary>
/// <param name="A">Matrix A</param>
/// <param name="row">�к�</param>
/// <param name="col">�к�</param>
/// <returns>���ظ�λ�ø���ֵ</returns>
__device__ float GetElement(const Matrix A, int row, int col)
{
	return A.elements[row * A.stride + col];
}
/// <summary>
/// ���ľ���Ԫ��
/// </summary>
/// <param name="A">Matrix A</param>
/// <param name="row">�к�</param>
/// <param name="col">�к�</param>
/// <param name="value">�޸�ֵ</param>
__device__ void SetElement(Matrix A, int row, int col, float value)
{
	A.elements[row * A.stride + col] = value;
}
/// <summary>
/// �õ��Ӿ���
/// </summary>
/// <param name="A">ĸ����A</param>
/// <param name="row">�к�</param>
/// <param name="col">�к�</param>
/// <returns>Matrix �Ӿ���</returns>
__device__ Matrix GetSubMatrix(Matrix A, int row, int col)
{
	Matrix Asub;
	Asub.width = BLOCK_SIZE;
	Asub.height = BLOCK_SIZE;
	Asub.stride = A.stride;
	Asub.elements = &A.elements[A.stride * BLOCK_SIZE * row + BLOCK_SIZE * col];
	return Asub;
}
__device__ int calculateDimGridDevice(int a, int b) {
	float A = float(a);
	float B = float(b);
	float C = (A + (B - 1)) / B;
	return int(C);
}
/// <summary>
/// ������ģ������ڴ�汾
/// </summary>
__global__ void MatMulKernelV2(Matrix A, Matrix B, Matrix C)
{
	// �߳̿���к���
	int blockRow = blockIdx.y;
	int blockCol = blockIdx.x;
	// ÿ���߳̿����C��һ��
	Matrix Csub = GetSubMatrix(C, blockRow, blockCol);
	// �߳̿��е�ÿ���̼߳�����߳̿��е�һ��Ԫ��
	float Cvalue = 0.0f;  //��Ԫ�ص�ֵ
	// Ԫ�����߳̿��е��к���
	int row = threadIdx.y;
	int col = threadIdx.x;
	//��C�����е�λ��
	int row_all = blockRow * BLOCK_SIZE + row;
	int col_all = blockCol * BLOCK_SIZE + col;
	//�趨�����ڴ�
	__shared__ float As[BLOCK_SIZE][BLOCK_SIZE];
	__shared__ float Bs[BLOCK_SIZE][BLOCK_SIZE];
	for (int m = 0; m < calculateDimGridDevice(A.width, BLOCK_SIZE); ++m) {
		//Matrix Asub = GetSubMatrix(A, blockRow, m);
		//Matrix Bsub = GetSubMatrix(B, m, blockCol);		
		//�߳̿��е�ÿ���̶߳�ȡһ��Ԫ��
		if (m * BLOCK_SIZE + col < A.width && row_all < A.height)
			//As[row][col] = GetElement(Asub, row, col);
			As[row][col] = A.elements[row_all * A.width + m * BLOCK_SIZE + col];
		else
			As[row][col] = 0.0f;

		if (m * BLOCK_SIZE + row < B.height && col_all < B.width)
			//Bs[row][col] = GetElement(Asub, row, col);
			Bs[row][col] = B.elements[(m * BLOCK_SIZE + row) * B.width + col_all];
		else
			Bs[row][col] = 0.0f;
		//�߳�ͬ�����ȴ������߳���ɶ�ȡ����
		__syncthreads();
		for (int e = 0; e < BLOCK_SIZE; ++e)
			Cvalue += As[row][e] * Bs[e][col];
		//�ڽ�����һ����֮ǰ��ȷ��ÿ���̶߳������֮ǰ�Ĳ���
		__syncthreads();
		//������ó���ֵ�洢��ȫ���ڴ���
		if (row_all < C.height && col_all < C.width)
			SetElement(Csub, row, col, Cvalue);
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// CUDA����˷��������ڴ淽��
/// C = A �� B
/// </summary>
/// <param name="A">������A</param>
/// <param name="B">������B</param>
/// <param name="C">������C</param>
/// <param name="times">�������</param>
/// <param name="warmupTimes">Ԥ�ȴ���</param>
/// <returns>��������ʱ�䣨��λ�����룩</returns>
float matrixMulCudaV2(const Matrix A, const Matrix B, Matrix C, int times, int warmupTimes = 10)
{
	float start, end, diffTime;
	//���ؾ����Դ�
	Matrix d_A;
	d_A.width = A.width; d_A.height = A.height; d_A.stride = A.stride;
	size_t size = A.width * A.height * sizeof(float);
	cudaMalloc(&d_A.elements, size);
	cudaMemcpy(d_A.elements, A.elements, size,
		cudaMemcpyHostToDevice);
	Matrix d_B;
	d_B.width = B.width; d_B.height = B.height; d_B.stride = B.stride;
	size = B.width * B.height * sizeof(float);
	cudaMalloc(&d_B.elements, size);
	cudaMemcpy(d_B.elements, B.elements, size,
		cudaMemcpyHostToDevice);

	Matrix d_C;
	d_C.width = C.width; d_C.height = C.height; d_C.stride = C.stride;
	size = C.width * C.height * sizeof(float);
	cudaMalloc(&d_C.elements, size);

	dim3 dimBlock(BLOCK_SIZE, BLOCK_SIZE);
	//dim3 dimGrid(B.width / dimBlock.x, A.height / dimBlock.y);
	dim3 dimGrid(calculateDimGrid(B.width, dimBlock.x), calculateDimGrid(A.height, dimBlock.y));
	for (int num = 0; num < warmupTimes; ++num) {
		MatMulKernelV2 << <dimGrid, dimBlock >> > (d_A, d_B, d_C);
		cudaDeviceSynchronize();
	}//Ԥ��������ø��Ӿ�ȷ�ļ�ʱ
	start = clock();
	for (int num = 0; num < times; ++num) {
		MatMulKernelV2 << <dimGrid, dimBlock >> > (d_A, d_B, d_C);
		cudaDeviceSynchronize();
	}
	end = clock();
	//���ؾ���C���ڴ�
	cudaMemcpy(C.elements, d_C.elements, size,
		cudaMemcpyDeviceToHost);

	diffTime = end - start;
	//printMatrix(C);
	//�ͷſռ�
	cudaFree(d_A.elements);
	cudaFree(d_B.elements);
	cudaFree(d_C.elements);
	return diffTime / float(times);
}