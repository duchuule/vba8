#include "pch.h"
#include "Renderer.h"
#include "EmulatorFileHandler.h"
#include "Vector4.h"
#include "TextureLoader.h"
#include "WP8VBAMComponent.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Windows::Graphics::Display;

#define CROSS_TEXTURE_FILE_NAME						L"Assets/pad_cross.dds"
#define BUTTONS_LANDSCAPE_TEXTURE_FILE_NAME			L"Assets/pad_buttons_landscape.dds"
#define BUTTONS_PORTRAIT_TEXTURE_FILE_NAME			L"Assets/pad_buttons_portrait.dds"
#define SS_TEXTURE_FILE_NAME						L"Assets/pad_start_select.dds"
#define L_TEXTURE_FILE_NAME							L"Assets/pad_l_button.dds"
#define R_TEXTURE_FILE_NAME							L"Assets/pad_r_button.dds"
#define STICK_TEXTURE_FILE_NAME						L"Assets/ThumbStick.dds"
#define STICK_CENTER_TEXTURE_FILE_NAME				L"Assets/ThumbStickCenter.dds"

#define AUTOSAVE_INTERVAL			60.0f


Renderer::Renderer()
{ 
}

Renderer::~Renderer(void)
{
}

void Renderer::CreateDeviceResources()
{
	Direct3DBase::CreateDeviceResources();
	
	this->m_d3dDevice->GetImmediateContext1(this->m_d3dContext.GetAddressOf());

	LoadTextureFromFile(
		this->m_d3dDevice.Get(), 
		STICK_TEXTURE_FILE_NAME,
		this->stickResource.GetAddressOf(), 
		this->stickSRV.GetAddressOf()
		);

	LoadTextureFromFile(
		this->m_d3dDevice.Get(), 
		STICK_CENTER_TEXTURE_FILE_NAME,
		this->stickCenterResource.GetAddressOf(), 
		this->stickCenterSRV.GetAddressOf()
		);

	LoadTextureFromFile(
		this->m_d3dDevice.Get(), 
		CROSS_TEXTURE_FILE_NAME,
		this->crossResource.GetAddressOf(), 
		this->crossSRV.GetAddressOf()
		);

	LoadTextureFromFile(
		this->m_d3dDevice.Get(), 
		BUTTONS_LANDSCAPE_TEXTURE_FILE_NAME,
		this->buttonsLandscapeResource.GetAddressOf(), 
		this->buttonsLandscapeSRV.GetAddressOf()
		);

	LoadTextureFromFile(
		this->m_d3dDevice.Get(), 
		BUTTONS_PORTRAIT_TEXTURE_FILE_NAME,
		this->buttonsPortraitResource.GetAddressOf(), 
		this->buttonsPortraitSRV.GetAddressOf()
		);

	LoadTextureFromFile(
		this->m_d3dDevice.Get(), 
		SS_TEXTURE_FILE_NAME,
		this->startSelectResource.GetAddressOf(), 
		this->startSelectSRV.GetAddressOf()
		);

	LoadTextureFromFile(
		this->m_d3dDevice.Get(), 
		L_TEXTURE_FILE_NAME,
		this->lButtonResource.GetAddressOf(), 
		this->lButtonSRV.GetAddressOf()
		);

	LoadTextureFromFile(
		this->m_d3dDevice.Get(), 
		R_TEXTURE_FILE_NAME,
		this->rButtonResource.GetAddressOf(), 
		this->rButtonSRV.GetAddressOf()
		);

	// Create Textures and SRVs for front and backbuffer
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.Format = DXGI_FORMAT_B8G8R8X8_UNORM;
	desc.Width = 241;
	desc.Height = 162;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DYNAMIC;

	DX::ThrowIfFailed(
		this->m_d3dDevice->CreateTexture2D(&desc, nullptr, this->buffers[0].GetAddressOf())
		);
	DX::ThrowIfFailed(
		this->m_d3dDevice->CreateTexture2D(&desc, nullptr, this->buffers[1].GetAddressOf())
		);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	DX::ThrowIfFailed(
		this->m_d3dDevice->CreateShaderResourceView(this->buffers[0].Get(), &srvDesc, this->bufferSRVs[0].GetAddressOf())
		);
	DX::ThrowIfFailed(
		this->m_d3dDevice->CreateShaderResourceView(this->buffers[1].Get(), &srvDesc, this->bufferSRVs[1].GetAddressOf())
		);


	

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));

	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	DX::ThrowIfFailed(
		this->m_d3dDevice->CreateBlendState(&blendDesc, this->alphablend.GetAddressOf())
		);
}

void Renderer::CreateWindowSizeDependentResources()
{
	Direct3DBase::CreateWindowSizeDependentResources();
}

void Renderer::UpdateForWindowSizeChange(float width, float height)
{
	Direct3DBase::UpdateForWindowSizeChange(width, height);

	float scale = ((int)DisplayProperties::ResolutionScale) / 100.0f;
	this->height = width * scale;
	this->width = height * scale;

	if(!this->dxSpriteBatch)
	{
		this->dxSpriteBatch = new DXSpriteBatch(this->m_d3dDevice.Get(), this->m_d3dContext.Get(), this->height, this->width);
	}else
	{
		this->dxSpriteBatch->OnResize(this->height, this->width);
	}

	if(this->height > 700.0f && this->height < 740.0f)
	{
		this->format = HD720P;
	}else if(this->height > 760.0f)
	{
		this->format = WXGA;
	}else 
	{
		this->format = WVGA;
	}


	this->CreateTransformMatrix();
}




void Renderer::CreateTransformMatrix(void)
{
	this->outputTransform = XMMatrixIdentity();

	if(this->orientation != ORIENTATION_PORTRAIT)
	{
		if(this->orientation == ORIENTATION_LANDSCAPE_RIGHT)
		{
			this->outputTransform = XMMatrixMultiply(XMMatrixTranslation(-this->width/2, -this->height/2, 0.0f), XMMatrixRotationZ(XM_PI));
			this->outputTransform = XMMatrixMultiply(this->outputTransform, XMMatrixTranslation(this->width/2, this->height/2, 0.0f));
		}

		this->outputTransform = XMMatrixMultiply(this->outputTransform, XMMatrixRotationZ(XM_PIDIV2));
		this->outputTransform = XMMatrixMultiply(this->outputTransform, XMMatrixTranslation(this->height, 0.0f, 0.0f));
	}
}

void Renderer::Update(float timeTotal, float timeDelta)
{
}

void Renderer::Render()
{
}

