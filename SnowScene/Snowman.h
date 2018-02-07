//***************************************************************************************
// Snowman.h
// 
// Simple class for rendering a snowman.
//***************************************************************************************

#ifndef SNOWMAN_H
#define SNOWMAN_H

#include "d3dUtil.h"

class Camera;

class Snowman
{
public:
	Snowman(ID3D11Device* device, FLOAT snowmanScale);
	~Snowman();

	void Init();
	void UpdatePosition(XMMATRIX base);
	void Draw(ID3D11DeviceContext* dc, const Camera& camera);

private:
	Snowman(const Snowman& rhs);
	Snowman& operator=(const Snowman& rhs);
	void BuildSnowmanBuffers();
private:
	ID3D11Device * md3dDevice;

	ID3D11Buffer* mVB;
	ID3D11Buffer* mIB;

	ID3D11ShaderResourceView* mSnowmanTexSRV;
	ID3D11ShaderResourceView* mBlackTexSRV;
	ID3D11ShaderResourceView* mRedTexSRV;
	ID3D11ShaderResourceView* mHandTexSRV;

	Material mBodySphereMat;
	Material mHeadSphereMat;
	Material mHatEdgeCylinderMat;
	Material mHatCylinderMat;
	Material mEyeSphereMat;
	Material mNoseCylinderMat;
	Material mHandCylinderMat;
	Material mMouthBoxMat;

	// Define transformations from local spaces to world space.
	XMFLOAT4X4 mBodySphereWorld;
	XMFLOAT4X4 mHeadSphereWorld;
	XMFLOAT4X4 mHatEdgeCylinderWorld;
	XMFLOAT4X4 mHatCylinderWorld;
	XMFLOAT4X4 mEyeSphereWorld[2];
	XMFLOAT4X4 mNoseCylinderWorld;
	XMFLOAT4X4 mHandCylinderWorld;
	XMFLOAT4X4 mMouthBoxWorld;

	int mBoxVertexOffset;
	int mBodySphereVertexOffset;
	int mHeadSphereVertexOffset;
	int mHatEdgeCylinderVertexOffset;
	int mHatCylinderVertexOffset;
	int mEyeSphereVertexOffset;
	int mNoseCylinderVertexOffset;
	int mHandCylinderVertexOffset;
	int mMouthBoxVertexOffset;

	UINT mBoxIndexOffset;
	UINT mBodySphereIndexOffset;
	UINT mHeadSphereIndexOffset;
	UINT mHatEdgeCylinderIndexOffset;
	UINT mHatCylinderIndexOffset;
	UINT mEyeSphereIndexOffset;
	UINT mNoseCylinderIndexOffset;
	UINT mHandCylinderIndexOffset;
	UINT mMouthBoxIndexOffset;

	UINT mBoxIndexCount;
	UINT mBodySphereIndexCount;
	UINT mHeadSphereIndexCount;
	UINT mHatEdgeCylinderIndexCount;
	UINT mHatCylinderIndexCount;
	UINT mEyeSphereIndexCount;
	UINT mNoseCylinderIndexCount;
	UINT mHandCylinderIndexCount;
	UINT mMouthBoxIndexCount;

	FLOAT mX;
	FLOAT mY;
	FLOAT mZ;
	FLOAT mBoxScale;
	FLOAT mBodyScale;
	FLOAT mHeadScale;
	FLOAT mHatEdgeRadius;
	FLOAT mHatEdgeHeight;
	FLOAT mHatRadius;
	FLOAT mHatHeight;
	FLOAT mEyeScale;
	FLOAT mNoseRadius;
	FLOAT mNoseHeight;
	FLOAT mHandRadius;
	FLOAT mHandHeight;
	FLOAT mMouthWidth;
	FLOAT mMouthHeight;
	FLOAT mMouthDepth;
};

#endif // SNOWMAN_H