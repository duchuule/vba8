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




Renderer::Renderer()
{ 
}

Renderer::~Renderer(void)
{
}

void Renderer::DrawController(void)
{


	//A-B button
	Engine::Rectangle aRect (this->aRectangle.left, this->aRectangle.top, this->aRectangle.right - this->aRectangle.left, this->aRectangle.bottom - this->aRectangle.top);
	ComPtr<ID3D11Texture2D> tex;
	this->aResource.As(&tex);
	this->dxSpriteBatch->Draw(aRect, this->aSRV.Get(), tex.Get(), a_color);

	Engine::Rectangle bRect (this->bRectangle.left, this->bRectangle.top, this->bRectangle.right - this->bRectangle.left, this->bRectangle.bottom - this->bRectangle.top);
	ComPtr<ID3D11Texture2D> tex2;
	this->bResource.As(&tex2);
	this->dxSpriteBatch->Draw(bRect, this->bSRV.Get(), tex2.Get(), b_color);

	if(pad_to_draw == 0)
	{
		Engine::Rectangle crossRect (this->crossRectangle.left, this->crossRectangle.top, this->crossRectangle.right - this->crossRectangle.left, this->crossRectangle.bottom - this->crossRectangle.top);

		ComPtr<ID3D11Texture2D> tex;
		this->crossResource.As(&tex);
		this->dxSpriteBatch->Draw(crossRect, this->crossSRV.Get(), tex.Get(), joystick_color);
	}else if(pad_to_draw == 1)
	{

		Engine::Rectangle stickRectE (stickRect.left, stickRect.top, stickRect.right - stickRect.left, stickRect.bottom - stickRect.top);
		Engine::Rectangle stickRectCenterE (centerRect.left, centerRect.top, centerRect.right - centerRect.left, centerRect.bottom - centerRect.top);

		ComPtr<ID3D11Texture2D> tex;
		this->stickResource.As(&tex);
		ComPtr<ID3D11Texture2D> tex2;
		this->stickCenterResource.As(&tex2);
		this->dxSpriteBatch->Draw(stickRectCenterE, this->stickCenterSRV.Get(), tex2.Get(), joystick_center_color);
		this->dxSpriteBatch->Draw(stickRectE, this->stickSRV.Get(), tex.Get(), joystick_color);
	}

	//start select buttons
	Engine::Rectangle startRectE (startRectangle.left, startRectangle.top, startRectangle.right - startRectangle.left, startRectangle.bottom - startRectangle.top);
	ComPtr<ID3D11Texture2D> texS;
	this->startResource.As(&texS);
	this->dxSpriteBatch->Draw(startRectE, this->startSRV.Get(), texS.Get(), start_color);

	Engine::Rectangle selectRectE (selectRectangle.left, selectRectangle.top, selectRectangle.right - selectRectangle.left, selectRectangle.bottom - selectRectangle.top);
	ComPtr<ID3D11Texture2D> texSS;
	this->selectResource.As(&texSS);
	this->dxSpriteBatch->Draw(selectRectE, this->selectSRV.Get(), texSS.Get(), select_color);


	//L-R buttons
	if(should_draw_LR)
	{
		Engine::Rectangle lRectE (lRectangle.left, lRectangle.top, lRectangle.right - lRectangle.left, lRectangle.bottom - lRectangle.top);
		Engine::Rectangle rRectE (rRectangle.left, rRectangle.top, rRectangle.right - rRectangle.left, rRectangle.bottom - rRectangle.top);

		ComPtr<ID3D11Texture2D> tex;
		this->lButtonResource.As(&tex);
		ComPtr<ID3D11Texture2D> tex2;
		this->rButtonResource.As(&tex2);
		this->dxSpriteBatch->Draw(lRectE, this->lButtonSRV.Get(), tex.Get(), l_color);
		this->dxSpriteBatch->Draw(rRectE, this->rButtonSRV.Get(), tex2.Get(), r_color);
	}

}
void Renderer::CreateDeviceResources()
{
	Direct3DBase::CreateDeviceResources();
	
	this->m_d3dDevice->GetImmediateContext1(this->m_d3dContext.GetAddressOf());

	LoadTextureFromFile(
			this->m_d3dDevice.Get(), 
			DIVIDER_TEXTURE_FILE_NAME,
			this->dividerResource.GetAddressOf(), 
			this->dividerSRV.GetAddressOf()
			);

	if (this->settings->UseColorButtons)
	{
		LoadTextureFromFile(
			this->m_d3dDevice.Get(), 
			STICK_COLOR_TEXTURE_FILE_NAME,
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
			CROSS_COLOR_TEXTURE_FILE_NAME,
			this->crossResource.GetAddressOf(), 
			this->crossSRV.GetAddressOf()
			);

		LoadTextureFromFile(
			this->m_d3dDevice.Get(), 
			A_COLOR_TEXTURE_FILE_NAME,
			this->aResource.GetAddressOf(), 
			this->aSRV.GetAddressOf()
			);

		LoadTextureFromFile(
			this->m_d3dDevice.Get(), 
			B_COLOR_TEXTURE_FILE_NAME,
			this->bResource.GetAddressOf(), 
			this->bSRV.GetAddressOf()
			);



		LoadTextureFromFile(
			this->m_d3dDevice.Get(), 
			START_COLOR_TEXTURE_FILE_NAME,
			this->startResource.GetAddressOf(), 
			this->startSRV.GetAddressOf()
			);

		LoadTextureFromFile(
			this->m_d3dDevice.Get(), 
			SELECT_COLOR_TEXTURE_FILE_NAME,
			this->selectResource.GetAddressOf(), 
			this->selectSRV.GetAddressOf()
			);

		LoadTextureFromFile(
			this->m_d3dDevice.Get(), 
			L_COLOR_TEXTURE_FILE_NAME,
			this->lButtonResource.GetAddressOf(), 
			this->lButtonSRV.GetAddressOf()
			);

		LoadTextureFromFile(
			this->m_d3dDevice.Get(), 
			R_COLOR_TEXTURE_FILE_NAME,
			this->rButtonResource.GetAddressOf(), 
			this->rButtonSRV.GetAddressOf()
			);
	}
	else
	{
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
			A_TEXTURE_FILE_NAME,
			this->aResource.GetAddressOf(), 
			this->aSRV.GetAddressOf()
			);

		LoadTextureFromFile(
			this->m_d3dDevice.Get(), 
			B_TEXTURE_FILE_NAME,
			this->bResource.GetAddressOf(), 
			this->bSRV.GetAddressOf()
			);



		LoadTextureFromFile(
			this->m_d3dDevice.Get(), 
			START_TEXTURE_FILE_NAME,
			this->startResource.GetAddressOf(), 
			this->startSRV.GetAddressOf()
			);

		LoadTextureFromFile(
			this->m_d3dDevice.Get(), 
			SELECT_TEXTURE_FILE_NAME,
			this->selectResource.GetAddressOf(), 
			this->selectSRV.GetAddressOf()
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
	}
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

