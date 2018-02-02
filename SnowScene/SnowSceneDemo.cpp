//***************************************************************************************
// SnowSceneDemo.cpp by huangjw 2017 All Rights Reserved.
//
// Demonstrates dynamic reflections with dynamic cube maps.
//
// Controls:
//		Hold the left mouse button down and move the mouse to rotate.
//      Hold the right mouse button down to zoom in and out.
//      Press '1', '2', '3' for 1, 2, or 3 lights enabled.
//
//***************************************************************************************

#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "Effects.h"
#include "Vertex.h"
#include "Camera.h"
#include "Sky.h"
#include "Snowman.h"
#include "RenderStates.h"
#include "ParticleSystem.h"
#include "Terrain.h"

class SnowSceneApp : public D3DApp
{
public:
	SnowSceneApp(HINSTANCE hInstance);
	~SnowSceneApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void DrawScene(const Camera& camera, bool drawSkull);
	void BuildShapeGeometryBuffers();

private:

	Sky * mSky;
	Terrain mTerrain;

	ParticleSystem mSnow;

	bool mWalkCamMode;

	Snowman* mSnowmanBox;
	Snowman* mSnowmanFloor;

	ID3D11Buffer* mShapesVB;
	ID3D11Buffer* mShapesIB;

	ID3D11ShaderResourceView* mBoxTexSRV;
	ID3D11ShaderResourceView* mSnowTexSRV;
	ID3D11ShaderResourceView* mRandomTexSRV;

	DirectionalLight mDirLights[3];

	Material mBoxMat;

	// Define transformations from local spaces to world space.
	XMFLOAT4X4 mBoxWorld;

	int mBoxVertexOffset;
	int mFloorVertexOffset;

	UINT mBoxIndexOffset;

	UINT mBoxIndexCount;

	Camera mCam;
	//Camera mCubeMapCamera[6];

	POINT mLastMousePos;

	FLOAT mLeftX;
	FLOAT mRightX;
	FLOAT mZ;
	FLOAT mBoxScale;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	SnowSceneApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}


SnowSceneApp::SnowSceneApp(HINSTANCE hInstance)
	: D3DApp(hInstance), mSky(0),
	mSnowmanBox(0), mSnowmanFloor(0),
	mShapesVB(0), mShapesIB(0),
	mBoxTexSRV(0), mRandomTexSRV(0), mSnowTexSRV(0), mWalkCamMode(false)
{
	mMainWndCaption = L"Snow Scene Demo";
	mEnable4xMsaa = false;

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	mLeftX = -2.0f;
	mRightX = 5.0f;
	mZ = 120.0f;
	mBoxScale = 2.0f;

	mCam.SetPosition(0.0f, 2.0f, mZ - 20.0f);

	mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	mBoxMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mBoxMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
}

SnowSceneApp::~SnowSceneApp()
{
	md3dImmediateContext->ClearState();

	ReleaseCOM(mShapesVB);
	ReleaseCOM(mShapesIB);
	ReleaseCOM(mBoxTexSRV);
	ReleaseCOM(mRandomTexSRV);
	ReleaseCOM(mSnowTexSRV);

	SafeDelete(mSky);
	SafeDelete(mSnowmanBox);
	SafeDelete(mSnowmanFloor);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool SnowSceneApp::Init()
{
	if (!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);
	RenderStates::InitAll(md3dDevice);

	mSky = new Sky(md3dDevice, L"Textures/snowcube1024.dds", 5000.0f);

	Terrain::InitInfo tii;
	tii.HeightMapFilename = L"Textures/terrain.raw";
	tii.LayerMapFilename = L"Textures/layersnow.dds";
	tii.BlendMapFilename = L"Textures/blend.dds";
	tii.HeightScale = 50.0f;
	tii.HeightmapWidth = 2049;
	tii.HeightmapHeight = 2049;
	tii.CellSpacing = 0.5f;

	mTerrain.Init(md3dDevice, md3dImmediateContext, tii);

	mRandomTexSRV = d3dHelper::CreateRandomTexture1DSRV(md3dDevice);
	
	DirectX::CreateDDSTextureFromFile(md3dDevice, L"Textures/raindrop.dds", 0, &mSnowTexSRV);
	DirectX::CreateDDSTextureFromFile(md3dDevice, L"Textures/stone.dds", 0, &mBoxTexSRV);

	mSnow.Init(md3dDevice, Effects::SnowFX, mSnowTexSRV, mRandomTexSRV, 10000);

	mSnowmanBox = new Snowman(md3dDevice, mBoxScale);
	mSnowmanFloor = new Snowman(md3dDevice, mBoxScale);
	mSnowmanFloor->UpdateScene(XMMatrixTranslation(mRightX, 0, mZ));

	BuildShapeGeometryBuffers();

	return true;
}

void SnowSceneApp::OnResize()
{
	D3DApp::OnResize();

	mCam.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 3000.0f);
}

void SnowSceneApp::UpdateScene(float dt)
{
	// Control the camera.
	if (GetAsyncKeyState('W') & 0x8000)
		mCam.Walk(10.0f*dt);

	if (GetAsyncKeyState('S') & 0x8000)
		mCam.Walk(-10.0f*dt);

	if (GetAsyncKeyState('A') & 0x8000)
		mCam.Strafe(-10.0f*dt);

	if (GetAsyncKeyState('D') & 0x8000)
		mCam.Strafe(10.0f*dt);

	// Walk/fly mode
	if (GetAsyncKeyState('2') & 0x8000)
		mWalkCamMode = true;
	if (GetAsyncKeyState('3') & 0x8000)
		mWalkCamMode = false;

	// Clamp camera to terrain surface in walk mode.
	if (mWalkCamMode)
	{
		XMFLOAT3 camPos = mCam.GetPosition();
		float y = mTerrain.GetHeight(camPos.x, camPos.z);
		mCam.SetPosition(camPos.x, y + 2.0f, camPos.z);
	}

	// Reset particle systems.
	if (GetAsyncKeyState('R') & 0x8000)
	{
		mSnow.Reset();
	}

	// Rotate along the Y axis
	XMMATRIX localRotate = XMMatrixRotationY(-mTimer.TotalTime());
	XMMATRIX globalRotate = XMMatrixRotationY(mTimer.TotalTime());

	// Animate the box.
	XMMATRIX boxScale = XMMatrixScaling(mBoxScale, mBoxScale, mBoxScale);
	XMMATRIX boxOffset = XMMatrixTranslation(mLeftX, mBoxScale / 2, 0);
	XMMATRIX moveOffset = XMMatrixTranslation(0, 0, mZ);
	XMStoreFloat4x4(&mBoxWorld, boxScale * localRotate * boxOffset * globalRotate * moveOffset);

	// Animate the snowman on the box.
	mSnowmanBox->UpdateScene(localRotate * XMMatrixTranslation(mLeftX, mBoxScale, 0) * globalRotate * moveOffset);

	mSnow.Update(dt, mTimer.TotalTime());
	mCam.UpdateViewMatrix();
}

void SnowSceneApp::DrawScene()
{
	// Draw the scene as normal.
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	DrawScene(mCam, true);
	HR(mSwapChain->Present(0, 0));
}

void SnowSceneApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void SnowSceneApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void SnowSceneApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		mCam.Pitch(dy);
		mCam.RotateY(dx);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void SnowSceneApp::DrawScene(const Camera& camera, bool drawCenterSphere)
{
	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	XMMATRIX view = camera.View();
	XMMATRIX proj = camera.Proj();
	XMMATRIX viewProj = camera.ViewProj();

	// Set per frame constants.
	Effects::BasicFX->SetDirLights(mDirLights);
	Effects::BasicFX->SetEyePosW(mCam.GetPosition());

	// Figure out which technique to use.   

	ID3DX11EffectTechnique* activeTexTech = Effects::BasicFX->Light1TexTech;
	ID3DX11EffectTechnique* activeSkullTech = Effects::BasicFX->Light1Tech;
	ID3DX11EffectTechnique* activeReflectTech = Effects::BasicFX->Light1ReflectTech;
	
	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldViewProj;

	D3DX11_TECHNIQUE_DESC techDesc;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

	// Draw the box.
	activeTexTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Draw the box.
		world = XMLoadFloat4x4(&mBoxWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(mBoxMat);
		Effects::BasicFX->SetDiffuseMap(mBoxTexSRV);

		activeTexTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);
	}

	if (GetAsyncKeyState('1') & 0x8000)
		md3dImmediateContext->RSSetState(RenderStates::WireframeRS);

	mTerrain.Draw(md3dImmediateContext, mCam, mDirLights);
	md3dImmediateContext->RSSetState(0);
	mSky->Draw(md3dImmediateContext, camera);
	mSnowmanFloor->Draw(md3dImmediateContext, camera);
	mSnowmanBox->Draw(md3dImmediateContext, camera);

	// Draw particle systems last so it is blended with scene.
	mSnow.SetEyePos(mCam.GetPosition());
	mSnow.SetEmitPos(mCam.GetPosition());
	mSnow.Draw(md3dImmediateContext, mCam);

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	// restore default states.
	md3dImmediateContext->RSSetState(0);
	md3dImmediateContext->OMSetDepthStencilState(0, 0);
	md3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);
}

void SnowSceneApp::BuildShapeGeometryBuffers()
{
	//GeometryGenerator::MeshData floor;
	GeometryGenerator::MeshData box;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	mBoxVertexOffset = 0;

	// Cache the index count of each object.
	mBoxIndexCount = box.Indices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	mBoxIndexOffset = 0;

	UINT totalVertexCount = box.Vertices.size();

	UINT totalIndexCount = mBoxIndexCount;

	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	std::vector<Vertex::Basic32> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].Tex = box.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mShapesVB));

	// Pack the indices of all the meshes into one index buffer.
	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mShapesIB));
}