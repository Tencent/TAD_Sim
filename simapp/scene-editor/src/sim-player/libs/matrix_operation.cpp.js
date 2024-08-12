//#include "stdafx.h"
//#include "matrix_operation.h"

function matrix_operation () {}//js
//matrix_operation::matrix_operation()
//{
//}
//
//matrix_operation::~matrix_operation(void)
//{
//}

//�������
matrix_operation.prototype.mult_matrix = function (A, B,
  Result, m, p, n)//CPP2JS
{
  var i
  var j
  var k
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++) {
      Result[i * n + j] = 0.0
      for (k = 0; k < p; k++) {
        Result[i * n + j] += A[i * p + k] * B[k * n + j]
      }
    }
  }
}

matrix_operation.prototype.mult_matrix_333 = function (A, B, Result)//CPP2JS
{
  Result[0] = A[0] * B[0] + A[1] * B[3] + A[2] * B[6]
  Result[1] = A[0] * B[1] + A[1] * B[4] + A[2] * B[7]
  Result[2] = A[0] * B[2] + A[1] * B[5] + A[2] * B[8]
  Result[3] = A[3] * B[0] + A[4] * B[3] + A[5] * B[6]
  Result[4] = A[3] * B[1] + A[4] * B[4] + A[5] * B[7]
  Result[5] = A[3] * B[2] + A[4] * B[5] + A[5] * B[8]
  Result[6] = A[6] * B[0] + A[7] * B[3] + A[8] * B[6]
  Result[7] = A[6] * B[1] + A[7] * B[4] + A[8] * B[7]
  Result[8] = A[6] * B[2] + A[7] * B[5] + A[8] * B[8]
}

matrix_operation.prototype.mult_matrix_331 = function (A, B, Result)//CPP2JS
{
  Result[0] = A[0] * B[0] + A[1] * B[1] + A[2] * B[2]
  Result[1] = A[3] * B[0] + A[4] * B[1] + A[5] * B[2]
  Result[2] = A[6] * B[0] + A[7] * B[1] + A[8] * B[2]
}

//////////////////////////////////////////////////////////////////////////
//		��������
//
//		*A			������ķ���
//      *C          �����ķ���
//		 m			�����ά��
//
//////////////////////////////////////////////////////////////////////////
matrix_operation.prototype.reverse = function (A, C, m)//CPP2JS
{
  var i, j, x0, y0
  var M = 0
  var SP = new Array(m * m)
  var AB = new Array(m * m)
  var B = new Array(m * m)
  M = det(A, m)

  if (M == 0.0) {
    return
  }

  M = 1 / M
  for (i = 0; i < m; i++) {
    for (j = 0; j < m; j++) {
      for (x0 = 0; x0 < m; x0++) {
        for (y0 = 0; y0 < m; y0++) {
          B[x0 * m + y0] = A[x0 * m + y0]
        }
      }
      for (x0 = 0; x0 < m; x0++)
        B[x0 * m + j] = 0
      for (y0 = 0; y0 < m; y0++)
        B[i * m + y0] = 0
      B[i * m + j] = 1
      SP[i * m + j] = det(B, m)
      SP[i * m + j] = SP[i * m + j]
      AB[i * m + j] = M * SP[i * m + j]
    }
  }
  tran_matrix(AB, C, m, m)
  // delete []SP;
  // delete []AB;
  // delete []B;
  SP = NULL
  AB = NULL
  B = NULL
}

//////////////////////////////////////////////////////////////////////////
//		����ת��
//
//		*AT			ת�ú�ľ���
//		*A			��ת�õľ���
//		m, n		�����ά��
//
//////////////////////////////////////////////////////////////////////////
matrix_operation.prototype.tran_matrix = function (A, AT, m, n)//CPP2JS
{
  var i
  var j
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++) {
      AT[j * m + i] = A[i * n + j]
    }
  }
}

//���������ʽ
matrix_operation.prototype.det = function (A, m)//CPP2JS
{
  var i = 0, ii = 0, j = 0, jj = 0, k = 0, t = 0, tt = 1
  var det = 1, mk = 0
  var pA = new Array(m * m)
  var pB = new Array(m)
  for (i = 0; i < m; i++) {
    pB[i] = 0
    for (j = 0; j < m; j++) {
      pA[i * m + j] = A[i * m + j]
    }
  }
  for (k = 0; k < m; k++) {
    for (j = k; j < m; j++) {
      if (pA[k * m + j]) {
        for (i = 0; i < m; i++) {
          pB[i] = pA[i * m + k]
          pA[i * m + k] = pA[i * m + j]
          pA[i * m + j] = pB[i]
        }
        if (j - k) {
          tt = tt * (-1)
        }
        t = t + 1
        break
      }
    }
    if (t) {
      for (ii = k + 1; ii < m; ii++) {
        mk = (-1) * pA[ii * m + k] / pA[k * m + k]
        pA[ii * m + k] = 0
        for (jj = k + 1; jj < m; jj++) {
          pA[ii * m + jj] = pA[ii * m + jj] + mk * pA[k * m + jj]
        }
      }
      det = det * pA[k * m + k]
      t = 0
    } else {
      det = 0
      break
    }
  }
  det = det * tt
  // delete []pA;
  // delete []pB;
  pA = null
  pB = null
  return det
}

export default matrix_operation
