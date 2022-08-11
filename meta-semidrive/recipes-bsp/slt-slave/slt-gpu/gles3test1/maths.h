/*************************************************************************/ /*!
@File
@Title          Matrix maths
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef __MATHS_H__
#define __MATHS_H__

#ifdef __GNUC__
#define __internal __attribute__((visibility("hidden")))
#else
#define __internal
#endif

void myFrustum(float pMatrix[4][4], float left, float right, float bottom, float top, float zNear, float zFar);
void myPersp(float pMatrix[4][4], float fovy, float aspect, float zNear, float zFar);
void myTranslate(float pMatrix[4][4], float fX, float fY, float fZ);
void myScale(float pMatrix[4][4], float fX, float fY, float fZ);
void myRotate(float pMatrix[4][4], float fX, float fY, float fZ, float angle);
void myIdentity(float pMatrix[4][4]);
void myMultMatrix(float psRes[4][4], float psSrcA[4][4], float psSrcB[4][4]);
void myInvertTransposeMatrix(float pDstMatrix[3][3], float pSrcMatrix[4][4]);

#endif /* __MATHS_H__ */
