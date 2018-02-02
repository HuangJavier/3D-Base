//***************************************************************************************
// Snowman.cpp by huangjw 2017 All Rights Reserved.
//***************************************************************************************

#include "Snowman.h"
#include "GeometryGenerator.h"
#include "Camera.h"
#include "Vertex.h"
#include "Effects.h"

Snowman::Snowman(ID3D11Device* device, FLOAT snowmanScale)
{
	md3dDevice = device;
	mBodyScale = snowmanScale * 3.0f / 4.0f;
	mHeadScale = snowmanScale / 2.0f;
	mHatEdgeRadius = snowmanScale / 4.0f;
	mHatEdgeHeight = snowmanScale / 20.0f;
	mHatRadius = snowmanScale / 7.0f;
	mHatHeight = snowmanScale / 8.0f;
	mEyeScale = snowmanScale / 20.0f;
	mNoseRadius = snowmanScale / 20.0f;
	mNoseHeight = snowmanScale / 6.0f;
	mHandRadius = snowmanScale / 40.0f;
	mHandHeight = snowmanScale / 2.0f;
	mMouthWidth = snowmanScale / 8.0f;
	mMouthHeight = snowmanScale / 40.0f;
	mMouthDepth = snowmanScale / 20.0f;

	Init();
	BuildSnowmanBuffers();
}

void Snowman::Init()
{
	mBodySphereMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBodySphereMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBodySphereMat.Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mBodySphereMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mHeadSphereMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mHeadSphereMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mHeadSphereMat.Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mHeadSphereMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mHatEdgeCylinderMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mHatEdgeCylinderMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mHatEdgeCylinderMat.Specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);
	mHatEdgeCylinderMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mHatCylinderMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mHatCylinderMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mHatCylinderMat.Specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);
	mHatCylinderMat.Reflect = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);

	mEyeSphereMat.Ambient = XMFLOAT4(0.6f, 0.8f, 1.0f, 1.0f);
	mEyeSphereMat.Diffuse = XMFLOAT4(0.6f, 0.8f, 1.0f, 1.0f);
	mEyeSphereMat.Specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);
	mEyeSphereMat.Reflect = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);

	mNoseCylinderMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mNoseCylinderMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mNoseCylinderMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mNoseCylinderMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mHandCylinderMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mHandCylinderMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mHandCylinderMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mHandCylinderMat.Reflect = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);

	mMouthBoxMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mMouthBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mMouthBoxMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mMouthBoxMat.Reflect = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);

	ID3D11Resource* texResource = nullptr;

	HR(DirectX::CreateDDSTextureFromFile(md3dDevice,
		L"Textures/snow.dds", &texResource, &mSnowmanTexSRV));
	ReleaseCOM(texResource); // view saves reference

	HR(DirectX::CreateDDSTextureFromFile(md3dDevice,
		L"Textures/black.dds", &texResource, &mBlackTexSRV));
	ReleaseCOM(texResource); // view saves reference

	HR(DirectX::CreateDDSTextureFromFile(md3dDevice,
		L"Textures/red.dds", &texResource, &mRedTexSRV));
	ReleaseCOM(texResource); // view saves reference

	HR(DirectX::CreateDDSTextureFromFile(md3dDevice,
		L"Textures/hand.dds", &texResource, &mHandTexSRV));
	ReleaseCOM(texResource); // view saves reference
}


Snowman::~Snowman()
{
	ReleaseCOM(mVB);
	ReleaseCOM(mIB);
	ReleaseCOM(mSnowmanTexSRV);
	ReleaseCOM(mBlackTexSRV);
	ReleaseCOM(mRedTexSRV);
	ReleaseCOM(mHandTexSRV);
}


void Snowman::BuildSnowmanBuffers()
{
	GeometryGenerator::MeshData bodySphere;
	GeometryGenerator::MeshData headSphere;
	GeometryGenerator::MeshData hatEdgeCylinder;
	GeometryGenerator::MeshData hatCylinder;
	GeometryGenerator::MeshData eyeSphere;
	GeometryGenerator::MeshData noseCylinder;
	GeometryGenerator::MeshData handCylinder;
	GeometryGenerator::MeshData mouthBox;

	GeometryGenerator geoGen;
	geoGen.CreateSphere(mBodyScale / 2, 20, 20, bodySphere);
	geoGen.CreateSphere(mHeadScale / 2, 20, 20, headSphere);
	geoGen.CreateCylinder(mHatEdgeRadius, mHatEdgeRadius, mHatEdgeHeight, 20, 20, hatEdgeCylinder);
	geoGen.CreateCylinder(mHatRadius, mHatRadius, mHatHeight, 20, 20, hatCylinder);
	geoGen.CreateSphere(mEyeScale / 2, 20, 20, eyeSphere);
	geoGen.CreateCylinder(mNoseRadius, 0.0f, mNoseHeight, 20, 20, noseCylinder);
	geoGen.CreateCylinder(mHandRadius, mHandRadius, mHandHeight, 20, 20, handCylinder);
	geoGen.CreateBox(mMouthWidth, mMouthHeight, mMouthDepth, mouthBox);

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	mBodySphereVertexOffset = 0;
	mHeadSphereVertexOffset = mBodySphereVertexOffset + bodySphere.Vertices.size();
	mHatEdgeCylinderVertexOffset = mHeadSphereVertexOffset + headSphere.Vertices.size();
	mHatCylinderVertexOffset = mHatEdgeCylinderVertexOffset + hatEdgeCylinder.Vertices.size();
	mEyeSphereVertexOffset = mHatCylinderVertexOffset + hatCylinder.Vertices.size();
	mNoseCylinderVertexOffset = mEyeSphereVertexOffset + eyeSphere.Vertices.size();
	mHandCylinderVertexOffset = mNoseCylinderVertexOffset + noseCylinder.Vertices.size();
	mMouthBoxVertexOffset = mHandCylinderVertexOffset + handCylinder.Vertices.size();

	// Cache the index count of each object.
	mBodySphereIndexCount = bodySphere.Indices.size();
	mHeadSphereIndexCount = headSphere.Indices.size();
	mHatEdgeCylinderIndexCount = hatEdgeCylinder.Indices.size();
	mHatCylinderIndexCount = hatCylinder.Indices.size();
	mEyeSphereIndexCount = eyeSphere.Indices.size();
	mNoseCylinderIndexCount = noseCylinder.Indices.size();
	mHandCylinderIndexCount = handCylinder.Indices.size();
	mMouthBoxIndexCount = mouthBox.Indices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	mBodySphereIndexOffset = 0;
	mHeadSphereIndexOffset = mBodySphereIndexOffset + mBodySphereIndexCount;
	mHatEdgeCylinderIndexOffset = mHeadSphereIndexOffset + mHeadSphereIndexCount;
	mHatCylinderIndexOffset = mHatEdgeCylinderIndexOffset + mHatEdgeCylinderIndexCount;
	mEyeSphereIndexOffset = mHatCylinderIndexOffset + mHatCylinderIndexCount;
	mNoseCylinderIndexOffset = mEyeSphereIndexOffset + mEyeSphereIndexCount;
	mHandCylinderIndexOffset = mNoseCylinderIndexOffset + mNoseCylinderIndexCount;
	mMouthBoxIndexOffset = mHandCylinderIndexOffset + mHandCylinderIndexCount;

	UINT totalVertexCount =
		bodySphere.Vertices.size() +
		headSphere.Vertices.size() +
		hatEdgeCylinder.Vertices.size() +
		hatCylinder.Vertices.size() +
		eyeSphere.Vertices.size() +
		noseCylinder.Vertices.size() +
		handCylinder.Vertices.size() +
		mouthBox.Vertices.size();

	UINT totalIndexCount =
		mBodySphereIndexCount +
		mHeadSphereIndexCount +
		mHatEdgeCylinderIndexCount +
		mHatCylinderIndexCount +
		mEyeSphereIndexCount +
		mNoseCylinderIndexCount +
		mHandCylinderIndexCount +
		mMouthBoxIndexCount;

	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.

	std::vector<Vertex::Basic32> vertices(totalVertexCount);

	UINT k = 0;

	for (size_t i = 0; i < bodySphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = bodySphere.Vertices[i].Position;
		vertices[k].Normal = bodySphere.Vertices[i].Normal;
		vertices[k].Tex = bodySphere.Vertices[i].TexC;
	}

	for (size_t i = 0; i < headSphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = headSphere.Vertices[i].Position;
		vertices[k].Normal = headSphere.Vertices[i].Normal;
		vertices[k].Tex = headSphere.Vertices[i].TexC;
	}

	for (size_t i = 0; i < hatEdgeCylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = hatEdgeCylinder.Vertices[i].Position;
		vertices[k].Normal = hatEdgeCylinder.Vertices[i].Normal;
		vertices[k].Tex = hatEdgeCylinder.Vertices[i].TexC;
	}

	for (size_t i = 0; i < hatCylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = hatCylinder.Vertices[i].Position;
		vertices[k].Normal = hatCylinder.Vertices[i].Normal;
		vertices[k].Tex = hatCylinder.Vertices[i].TexC;
	}

	for (size_t i = 0; i < eyeSphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = eyeSphere.Vertices[i].Position;
		vertices[k].Normal = eyeSphere.Vertices[i].Normal;
		vertices[k].Tex = eyeSphere.Vertices[i].TexC;
	}

	for (size_t i = 0; i < noseCylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = noseCylinder.Vertices[i].Position;
		vertices[k].Normal = noseCylinder.Vertices[i].Normal;
		vertices[k].Tex = noseCylinder.Vertices[i].TexC;
	}

	for (size_t i = 0; i < handCylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = handCylinder.Vertices[i].Position;
		vertices[k].Normal = handCylinder.Vertices[i].Normal;
		vertices[k].Tex = handCylinder.Vertices[i].TexC;
	}

	for (size_t i = 0; i < mouthBox.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = mouthBox.Vertices[i].Position;
		vertices[k].Normal = mouthBox.Vertices[i].Normal;
		vertices[k].Tex = mouthBox.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));

	// Pack the indices of all the meshes into one index buffer.

	std::vector<UINT> indices;
	indices.insert(indices.end(), bodySphere.Indices.begin(), bodySphere.Indices.end());
	indices.insert(indices.end(), headSphere.Indices.begin(), headSphere.Indices.end());
	indices.insert(indices.end(), hatEdgeCylinder.Indices.begin(), hatEdgeCylinder.Indices.end());
	indices.insert(indices.end(), hatCylinder.Indices.begin(), hatCylinder.Indices.end());
	indices.insert(indices.end(), eyeSphere.Indices.begin(), eyeSphere.Indices.end());
	indices.insert(indices.end(), noseCylinder.Indices.begin(), noseCylinder.Indices.end());
	indices.insert(indices.end(), handCylinder.Indices.begin(), handCylinder.Indices.end());
	indices.insert(indices.end(), mouthBox.Indices.begin(), mouthBox.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mIB));
}


void Snowman::UpdateScene(XMMATRIX base)
{
	XMStoreFloat4x4(&mBodySphereWorld, XMMatrixTranslation(0.0f, mBodyScale / 2, 0.0f) * base);

	FLOAT headHeight = mBodyScale + mHeadScale * 3.0f / 8.0f;
	XMStoreFloat4x4(&mHeadSphereWorld, XMMatrixTranslation(0.0f, headHeight, 0.0f) * base);

	FLOAT hatEdgeHeight = headHeight + mHeadScale * 3.0f / 8.0f;
	XMStoreFloat4x4(&mHatEdgeCylinderWorld, XMMatrixTranslation(0.0f, hatEdgeHeight +
		mHatEdgeHeight, 0.0f) * base);

	XMStoreFloat4x4(&mHatCylinderWorld, XMMatrixTranslation(0.0f, hatEdgeHeight +
		mHatEdgeHeight + mHatHeight / 2, 0.0f) * base);

	XMStoreFloat4x4(&mEyeSphereWorld[0], XMMatrixTranslation(- mHeadScale / 2 * sinf(XM_PI / 6.0f),
		headHeight + mHeadScale / 2 * sinf(XM_PI / 6.0f), - mHeadScale / 2 * cosf(XM_PI / 6.0f)) * base);
	XMStoreFloat4x4(&mEyeSphereWorld[1], XMMatrixTranslation(mHeadScale / 2 * sinf(XM_PI / 6.0f),
		headHeight + mHeadScale / 2 * sinf(XM_PI / 6.0f), - mHeadScale / 2 * cosf(XM_PI / 6.0f)) * base);

	XMMATRIX noseLocalRotate = XMMatrixRotationX(XM_PI * 1.5f);
	XMMATRIX noseOffset = XMMatrixTranslation(0.0f, headHeight, - mHeadScale / 2 - mNoseHeight / 2);
	XMStoreFloat4x4(&mNoseCylinderWorld, noseLocalRotate * noseOffset * base);

	XMMATRIX handLocalRotate = XMMatrixRotationZ(XM_PI * 0.75f);
	XMMATRIX handOffset = XMMatrixTranslation(mBodyScale / 2, mBodyScale * 3.0f / 4.0f, 0.0f);
	XMStoreFloat4x4(&mHandCylinderWorld, handLocalRotate * handOffset * base);

	XMStoreFloat4x4(&mMouthBoxWorld, XMMatrixTranslation(0.0f, headHeight - mHeadScale / 2 * sinf(XM_PI / 6.0f), - mHeadScale / 2 * cosf(XM_PI / 6.0f)) * base);
}


void Snowman::Draw(ID3D11DeviceContext* dc, const Camera& camera)
{
	dc->IASetInputLayout(InputLayouts::Basic32);
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	XMMATRIX view = camera.View();
	XMMATRIX proj = camera.Proj();
	XMMATRIX viewProj = camera.ViewProj();

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldViewProj;

	D3DX11_TECHNIQUE_DESC techDesc;

	ID3DX11EffectTechnique* activeTexTech = Effects::BasicFX->Light1TexTech;

	dc->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	dc->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);

	// Draw the snowman
	activeTexTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Draw the bodySpheres.
		world = XMLoadFloat4x4(&mBodySphereWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(mBodySphereMat);
		Effects::BasicFX->SetDiffuseMap(mSnowmanTexSRV);

		activeTexTech->GetPassByIndex(p)->Apply(0, dc);
		dc->DrawIndexed(mBodySphereIndexCount, mBodySphereIndexOffset, mBodySphereVertexOffset);

		// Draw the headSpheres.
		world = XMLoadFloat4x4(&mHeadSphereWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(mHeadSphereMat);
		Effects::BasicFX->SetDiffuseMap(mSnowmanTexSRV);

		activeTexTech->GetPassByIndex(p)->Apply(0, dc);
		dc->DrawIndexed(mHeadSphereIndexCount, mHeadSphereIndexOffset, mHeadSphereVertexOffset);

		// Draw the hatEdgeCylinders.
		world = XMLoadFloat4x4(&mHatEdgeCylinderWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(mHatEdgeCylinderMat);
		Effects::BasicFX->SetDiffuseMap(mBlackTexSRV);

		activeTexTech->GetPassByIndex(p)->Apply(0, dc);
		dc->DrawIndexed(mHatEdgeCylinderIndexCount, mHatEdgeCylinderIndexOffset, mHatEdgeCylinderVertexOffset);

		// Draw the hatCylinders.
		world = XMLoadFloat4x4(&mHatCylinderWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(mHatCylinderMat);
		Effects::BasicFX->SetDiffuseMap(mBlackTexSRV);

		activeTexTech->GetPassByIndex(p)->Apply(0, dc);
		dc->DrawIndexed(mHatCylinderIndexCount, mHatCylinderIndexOffset, mHatCylinderVertexOffset);

		// Draw the eyeSpheres.
		for (int i = 0; i < 2; ++i)
		{
			world = XMLoadFloat4x4(&mEyeSphereWorld[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world * view*proj;

			Effects::BasicFX->SetWorld(world);
			Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BasicFX->SetWorldViewProj(worldViewProj);
			Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
			Effects::BasicFX->SetMaterial(mEyeSphereMat);
			Effects::BasicFX->SetDiffuseMap(mBlackTexSRV);

			activeTexTech->GetPassByIndex(p)->Apply(0, dc);
			dc->DrawIndexed(mEyeSphereIndexCount, mEyeSphereIndexOffset, mEyeSphereVertexOffset);
		}

		// Draw the noseCylinders.
		world = XMLoadFloat4x4(&mNoseCylinderWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(mNoseCylinderMat);
		Effects::BasicFX->SetDiffuseMap(mRedTexSRV);

		activeTexTech->GetPassByIndex(p)->Apply(0, dc);
		dc->DrawIndexed(mNoseCylinderIndexCount, mNoseCylinderIndexOffset, mNoseCylinderVertexOffset);

		// Draw the handCylinders.
		world = XMLoadFloat4x4(&mHandCylinderWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(mHandCylinderMat);
		Effects::BasicFX->SetDiffuseMap(mHandTexSRV);

		activeTexTech->GetPassByIndex(p)->Apply(0, dc);
		dc->DrawIndexed(mHandCylinderIndexCount, mHandCylinderIndexOffset, mHandCylinderVertexOffset);

		// Draw the mouthBox.
		world = XMLoadFloat4x4(&mMouthBoxWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(mMouthBoxMat);
		Effects::BasicFX->SetDiffuseMap(mBlackTexSRV);

		activeTexTech->GetPassByIndex(p)->Apply(0, dc);
		dc->DrawIndexed(mMouthBoxIndexCount, mMouthBoxIndexOffset, mMouthBoxVertexOffset);
	}

	// restore default states, as the SkyFX changes them in the effect file.
	dc->RSSetState(0);
	dc->OMSetDepthStencilState(0, 0);
}