//***************************************************************************************
//SnowSceneDemo.cpp by huangjw 2017 All Rights Reserved.
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
	void BuildCubeFaceCamera(float x, float y, float z);
	void BuildDynamicCubeMapViews();
	void BuildShapeGeometryBuffers();
	//void BuildSkullGeometryBuffers();

private:

	Sky * mSky;
	Snowman* mSnowmanBox;
	Snowman* mSnowmanFloor;

	ID3D11Buffer* mShapesVB;
	ID3D11Buffer* mShapesIB;

	ID3D11ShaderResourceView* mBoxTexSRV;
	ID3D11ShaderResourceView* mFloorTexSRV;

	ID3D11DepthStencilView* mDynamicCubeMapDSV;
	ID3D11RenderTargetView* mDynamicCubeMapRTV[6];
	ID3D11ShaderResourceView* mDynamicCubeMapSRV;
	D3D11_VIEWPORT mCubeMapViewport;

	static const int CubeMapSize = 256;

	DirectionalLight mDirLights[3];

	Material mBoxMat;
	Material mFloorMat;


	// Define transformations from local spaces to world space.
	XMFLOAT4X4 mBoxWorld;
	XMFLOAT4X4 mFloorWorld;

	int mBoxVertexOffset;
	int mFloorVertexOffset;

	UINT mBoxIndexOffset;
	UINT mFloorIndexOffset;

	UINT mBoxIndexCount;
	UINT mFloorIndexCount;

	UINT mLightCount;

	Camera mCam;
	Camera mCubeMapCamera[6];

	POINT mLastMousePos;

	FLOAT mLeftX;
	FLOAT mRightX;
	FLOAT mZ;
	FLOAT mBoxScale;
	FLOAT mFloorScale;
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
	mBoxTexSRV(0), mFloorTexSRV(0),
	mDynamicCubeMapDSV(0), mDynamicCubeMapSRV(0),
	mLightCount(3)
{
	mMainWndCaption = L"Snow Scene Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	mCam.SetPosition(0.0f, 5.0f, -30.0f);

	BuildCubeFaceCamera(0.0f, 2.0f, 0.0f);

	for (int i = 0; i < 6; ++i)
	{
		mDynamicCubeMapRTV[i] = 0;
	}

	mLeftX = -2.0f;
	mRightX = 5.0f;
	mZ = 0.0f;
	mBoxScale = 2.0f;
	mFloorScale = 500.0f;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mFloorWorld, I);

	//XMMATRIX floorScale = XMMatrixScaling(mFloorScale, 1.0f, mFloorScale);
	//XMMATRIX floorOffset = XMMatrixIdentity();
	//XMStoreFloat4x4(&mFloorWorld, XMMatrixMultiply(floorScale, floorOffset));


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

	mFloorMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mFloorMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mFloorMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mFloorMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
}

SnowSceneApp::~SnowSceneApp()
{
	SafeDelete(mSky);
	SafeDelete(mSnowmanBox);
	SafeDelete(mSnowmanFloor);
	ReleaseCOM(mShapesVB);
	ReleaseCOM(mShapesIB);
	ReleaseCOM(mBoxTexSRV);
	ReleaseCOM(mFloorTexSRV);
	ReleaseCOM(mDynamicCubeMapDSV);
	ReleaseCOM(mDynamicCubeMapSRV);
	for (int i = 0; i < 6; ++i)
		ReleaseCOM(mDynamicCubeMapRTV[i]);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
}

bool SnowSceneApp::Init()
{
	if (!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);

	mSky = new Sky(md3dDevice, L"Textures/snowcube1024.dds", 5000.0f);
	mSnowmanBox = new Snowman(md3dDevice, mBoxScale);
	mSnowmanFloor = new Snowman(md3dDevice, mBoxScale);
	mSnowmanFloor->UpdateScene(XMMatrixTranslation(mRightX, 0, mZ));
	ID3D11Resource* texResource = nullptr;

	HR(DirectX::CreateDDSTextureFromFile(md3dDevice,
		L"Textures/stone.dds", &texResource, &mBoxTexSRV));
	ReleaseCOM(texResource); // view saves reference

	HR(DirectX::CreateDDSTextureFromFile(md3dDevice,
		L"Textures/snow.dds", &texResource, &mFloorTexSRV));
	ReleaseCOM(texResource); // view saves reference

	BuildDynamicCubeMapViews();

	BuildShapeGeometryBuffers();
	//BuildSkullGeometryBuffers();

	return true;
}

void SnowSceneApp::OnResize()
{
	D3DApp::OnResize();

	mCam.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
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

	// Switch the number of lights based on key presses.
	if (GetAsyncKeyState('0') & 0x8000)
		mLightCount = 0;

	if (GetAsyncKeyState('1') & 0x8000)
		mLightCount = 1;

	if (GetAsyncKeyState('2') & 0x8000)
		mLightCount = 2;

	if (GetAsyncKeyState('3') & 0x8000)
		mLightCount = 3;

	// Rotate along the Y axis
	XMMATRIX localRotate = XMMatrixRotationY(-mTimer.TotalTime());
	XMMATRIX globalRotate = XMMatrixRotationY(mTimer.TotalTime());

	// Animate the box.
	XMMATRIX boxScale = XMMatrixScaling(mBoxScale, mBoxScale, mBoxScale);
	XMMATRIX boxOffset = XMMatrixTranslation(mLeftX, mBoxScale / 2, mZ);
	XMStoreFloat4x4(&mBoxWorld, boxScale * localRotate * boxOffset * globalRotate);

	// Animate the snowman on the box.
	mSnowmanBox->UpdateScene(localRotate * XMMatrixTranslation(mLeftX, mBoxScale, mZ) * globalRotate);

	mCam.UpdateViewMatrix();
}

void SnowSceneApp::DrawScene()
{
	ID3D11RenderTargetView* renderTargets[1];

	// Generate the cube map.
	md3dImmediateContext->RSSetViewports(1, &mCubeMapViewport);
	for (int i = 0; i < 6; ++i)
	{
		// Clear cube map face and depth buffer.
		md3dImmediateContext->ClearRenderTargetView(mDynamicCubeMapRTV[i], reinterpret_cast<const float*>(&Colors::Silver));
		md3dImmediateContext->ClearDepthStencilView(mDynamicCubeMapDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// Bind cube map face as render target.
		renderTargets[0] = mDynamicCubeMapRTV[i];
		md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDynamicCubeMapDSV);

		// Draw the scene with the exception of the center sphere to this cube map face.
		DrawScene(mCubeMapCamera[i], false);
	}

	// Restore old viewport and render targets.
	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
	renderTargets[0] = mRenderTargetView;
	md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDepthStencilView);

	// Have hardware generate lower mipmap levels of cube map.
	md3dImmediateContext->GenerateMips(mDynamicCubeMapSRV);

	// Now draw the scene as normal, but with the center sphere.
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

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// Set per frame constants.
	Effects::BasicFX->SetDirLights(mDirLights);
	Effects::BasicFX->SetEyePosW(mCam.GetPosition());

	// Figure out which technique to use.   

	ID3DX11EffectTechnique* activeTexTech = Effects::BasicFX->Light1TexTech;
	ID3DX11EffectTechnique* activeSkullTech = Effects::BasicFX->Light1Tech;
	ID3DX11EffectTechnique* activeReflectTech = Effects::BasicFX->Light1ReflectTech;
	switch (mLightCount)
	{
	case 1:
		activeTexTech = Effects::BasicFX->Light1TexTech;
		activeSkullTech = Effects::BasicFX->Light1Tech;
		activeReflectTech = Effects::BasicFX->Light1ReflectTech;
		break;
	case 2:
		activeTexTech = Effects::BasicFX->Light2TexTech;
		activeSkullTech = Effects::BasicFX->Light2Tech;
		activeReflectTech = Effects::BasicFX->Light2ReflectTech;
		break;
	case 3:
		activeTexTech = Effects::BasicFX->Light3TexTech;
		activeSkullTech = Effects::BasicFX->Light3Tech;
		activeReflectTech = Effects::BasicFX->Light3ReflectTech;
		break;
	}

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldViewProj;

	D3DX11_TECHNIQUE_DESC techDesc;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

	// Draw the box and the floor.
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

		// Draw the floor.
		world = XMLoadFloat4x4(&mFloorWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixScaling(100.0f, 100.0f, 1.0f));
		Effects::BasicFX->SetMaterial(mFloorMat);
		Effects::BasicFX->SetDiffuseMap(mFloorTexSRV);

		activeTexTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mFloorIndexCount, mFloorIndexOffset, mFloorVertexOffset);
	}

	mSky->Draw(md3dImmediateContext, camera);
	mSnowmanFloor->Draw(md3dImmediateContext, camera);
	mSnowmanBox->Draw(md3dImmediateContext, camera);

	// restore default states, as the SkyFX changes them in the effect file.
	md3dImmediateContext->RSSetState(0);
	md3dImmediateContext->OMSetDepthStencilState(0, 0);
}

void SnowSceneApp::BuildCubeFaceCamera(float x, float y, float z)
{
	// Generate the cube map about the given position.
	XMFLOAT3 center(x, y, z);
	XMFLOAT3 worldUp(0.0f, 1.0f, 0.0f);

	// Look along each coordinate axis.
	XMFLOAT3 targets[6] =
	{
		XMFLOAT3(x + 1.0f, y, z), // +X
		XMFLOAT3(x - 1.0f, y, z), // -X
		XMFLOAT3(x, y + 1.0f, z), // +Y
		XMFLOAT3(x, y - 1.0f, z), // -Y
		XMFLOAT3(x, y, z + 1.0f), // +Z
		XMFLOAT3(x, y, z - 1.0f)  // -Z
	};

	// Use world up vector (0,1,0) for all directions except +Y/-Y.  In these cases, we
	// are looking down +Y or -Y, so we need a different "up" vector.
	XMFLOAT3 ups[6] =
	{
		XMFLOAT3(0.0f, 1.0f, 0.0f),  // +X
		XMFLOAT3(0.0f, 1.0f, 0.0f),  // -X
		XMFLOAT3(0.0f, 0.0f, -1.0f), // +Y
		XMFLOAT3(0.0f, 0.0f, +1.0f), // -Y
		XMFLOAT3(0.0f, 1.0f, 0.0f),	 // +Z
		XMFLOAT3(0.0f, 1.0f, 0.0f)	 // -Z
	};

	for (int i = 0; i < 6; ++i)
	{
		mCubeMapCamera[i].LookAt(center, targets[i], ups[i]);
		mCubeMapCamera[i].SetLens(0.5f*XM_PI, 1.0f, 0.1f, 1000.0f);
		mCubeMapCamera[i].UpdateViewMatrix();
	}
}

void SnowSceneApp::BuildDynamicCubeMapViews()
{
	//
	// Cubemap is a special texture array with 6 elements.
	//

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = CubeMapSize;
	texDesc.Height = CubeMapSize;
	texDesc.MipLevels = 0;
	texDesc.ArraySize = 6;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;

	ID3D11Texture2D* cubeTex = 0;
	HR(md3dDevice->CreateTexture2D(&texDesc, 0, &cubeTex));

	//
	// Create a render target view to each cube map face 
	// (i.e., each element in the texture array).
	// 

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.ArraySize = 1;
	rtvDesc.Texture2DArray.MipSlice = 0;

	for (int i = 0; i < 6; ++i)
	{
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		HR(md3dDevice->CreateRenderTargetView(cubeTex, &rtvDesc, &mDynamicCubeMapRTV[i]));
	}

	// Create a shader resource view to the cube map.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = -1;

	HR(md3dDevice->CreateShaderResourceView(cubeTex, &srvDesc, &mDynamicCubeMapSRV));

	ReleaseCOM(cubeTex);

	// We need a depth texture for rendering the scene into the cubemap
	// that has the same resolution as the cubemap faces.  
	D3D11_TEXTURE2D_DESC depthTexDesc;
	depthTexDesc.Width = CubeMapSize;
	depthTexDesc.Height = CubeMapSize;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.SampleDesc.Quality = 0;
	depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.MiscFlags = 0;

	ID3D11Texture2D* depthTex = 0;
	HR(md3dDevice->CreateTexture2D(&depthTexDesc, 0, &depthTex));

	// Create the depth stencil view for the entire cube
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = depthTexDesc.Format;
	dsvDesc.Flags = 0;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	HR(md3dDevice->CreateDepthStencilView(depthTex, &dsvDesc, &mDynamicCubeMapDSV));

	ReleaseCOM(depthTex);

	// Viewport for drawing into cubemap.
	mCubeMapViewport.TopLeftX = 0.0f;
	mCubeMapViewport.TopLeftY = 0.0f;
	mCubeMapViewport.Width = (float)CubeMapSize;
	mCubeMapViewport.Height = (float)CubeMapSize;
	mCubeMapViewport.MinDepth = 0.0f;
	mCubeMapViewport.MaxDepth = 1.0f;
}

void SnowSceneApp::BuildShapeGeometryBuffers()
{
	//GeometryGenerator::MeshData floor;
	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData floor;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(500.0f, 500.0f, 30, 30, floor);

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	mBoxVertexOffset = 0;
	mFloorVertexOffset = box.Vertices.size();


	// Cache the index count of each object.
	mBoxIndexCount = box.Indices.size();
	mFloorIndexCount =floor.Indices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	mBoxIndexOffset = 0;
	mFloorIndexOffset = mBoxIndexCount;

	UINT totalVertexCount =
		box.Vertices.size() +
		floor.Vertices.size();

	UINT totalIndexCount =
		mBoxIndexCount +
		mFloorIndexCount;

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

	for (size_t i = 0; i < floor.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = floor.Vertices[i].Position;
		vertices[k].Normal = floor.Vertices[i].Normal;
		vertices[k].Tex = floor.Vertices[i].TexC;
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
	indices.insert(indices.end(), floor.Indices.begin(), floor.Indices.end());

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

//void SnowSceneApp::BuildSkullGeometryBuffers()
//{
//	std::ifstream fin("Models/skull.txt");
//	
//	if(!fin)
//	{
//		MessageBox(0, L"Models/skull.txt not found.", 0, 0);
//		return;
//	}
//
//	UINT vcount = 0;
//	UINT tcount = 0;
//	std::string ignore;
//
//	fin >> ignore >> vcount;
//	fin >> ignore >> tcount;
//	fin >> ignore >> ignore >> ignore >> ignore;
//	
//	std::vector<Vertex::Basic32> vertices(vcount);
//	for(UINT i = 0; i < vcount; ++i)
//	{
//		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
//		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
//	}
//
//	fin >> ignore;
//	fin >> ignore;
//	fin >> ignore;
//
//	mSkullIndexCount = 3*tcount;
//	std::vector<UINT> indices(mSkullIndexCount);
//	for(UINT i = 0; i < tcount; ++i)
//	{
//		fin >> indices[i*3+0] >> indices[i*3+1] >> indices[i*3+2];
//	}
//
//	fin.close();
//
//    D3D11_BUFFER_DESC vbd;
//    vbd.Usage = D3D11_USAGE_IMMUTABLE;
//	vbd.ByteWidth = sizeof(Vertex::Basic32) * vcount;
//    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//    vbd.CPUAccessFlags = 0;
//    vbd.MiscFlags = 0;
//    D3D11_SUBRESOURCE_DATA vinitData;
//    vinitData.pSysMem = &vertices[0];
//    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mSkullVB));
//
//	//
//	// Pack the indices of all the meshes into one index buffer.
//	//
//
//	D3D11_BUFFER_DESC ibd;
//    ibd.Usage = D3D11_USAGE_IMMUTABLE;
//	ibd.ByteWidth = sizeof(UINT) * mSkullIndexCount;
//    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
//    ibd.CPUAccessFlags = 0;
//    ibd.MiscFlags = 0;
//    D3D11_SUBRESOURCE_DATA iinitData;
//	iinitData.pSysMem = &indices[0];
//    HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mSkullIB));
//}