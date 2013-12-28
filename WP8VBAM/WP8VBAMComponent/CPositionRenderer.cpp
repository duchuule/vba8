#include "pch.h"
#include "CPositionRenderer.h"
#include "Vector4.h"
#include "TextureLoader.h"
#include "CPositionComponent.h"

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





inline void cpyImg32( unsigned char *dst, unsigned int dstPitch, unsigned char *src, unsigned int srcPitch, unsigned short width, unsigned short height )
{
	// fast, iterative C version
	// copies an width*height array of visible pixels from src to dst
	// srcPitch and dstPitch are the number of garbage bytes after a scanline
	register unsigned short lineSize = width<<2;

	while( height-- ) {
		memcpy( dst, src, lineSize );
		src += srcPitch;
		dst += dstPitch;
	}
}

CPositionRenderer::CPositionRenderer()
	:frontbuffer(0), controller(nullptr),
	elapsedTime(0.0f), settings(EmulatorSettings::Current), frames(0)
{ 




}

CPositionRenderer::~CPositionRenderer(void)
{
	if(this->m_d3dContext)
	{
		this->m_d3dContext->Unmap(this->buffers[(this->frontbuffer + 1) % 2].Get(), 0);
	}


	delete this->dxSpriteBatch;
	this->dxSpriteBatch = nullptr;
}

void CPositionRenderer::CreateDeviceResources()
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

void CPositionRenderer::CreateWindowSizeDependentResources()
{
	Direct3DBase::CreateWindowSizeDependentResources();
}

void CPositionRenderer::UpdateForWindowSizeChange(float width, float height)
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

	if(this->controller)
	{
		this->controller->UpdateFormat(this->format);
	}

	this->CreateTransformMatrix();
}

void CPositionRenderer::UpdateController(void)
{
	if(this->controller)
	{
		this->controller->UpdateFormat(this->format);
	}
}


void CPositionRenderer::SetVirtualController(CPositionVirtualController *controller)
{
	this->controller = controller;
	this->controller->SetOrientation(this->orientation);
	this->controller->UpdateFormat(this->format);
	
}


void CPositionRenderer::ChangeOrientation(int orientation)
{
	this->orientation = orientation;
	if(this->controller)
	{
		this->controller->SetOrientation(this->orientation);
	}
	this->CreateTransformMatrix();
}

void CPositionRenderer::CreateTransformMatrix(void)
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

void CPositionRenderer::Update(float timeTotal, float timeDelta)
{
	cstate = controller->GetControllerState();
	

}

void CPositionRenderer::Render()
{


	m_d3dContext->OMSetRenderTargets(
		1,
		m_renderTargetView.GetAddressOf(),
		m_depthStencilView.Get()
		);

	const float fwhite[] = { 1.0f, 1.0f, 1.0f, 1.000f };
	m_d3dContext->ClearRenderTargetView(
		m_renderTargetView.Get(),
		fwhite
		);

	m_d3dContext->ClearDepthStencilView(
		m_depthStencilView.Get(),
		D3D11_CLEAR_DEPTH,
		1.0f,
		0
		);


	int height, width;
	RECT rect;


	this->controller->GetButtonsRectangle(&buttonsRectangle);
	this->controller->GetCrossRectangle(&crossRectangle);
	this->controller->GetStartSelectRectangle(&startSelectRectangle);
	this->controller->GetLRectangle(&lRectangle);
	this->controller->GetRRectangle(&rRectangle);

	float opacity = this->settings->ControllerOpacity / 100.0f;
	XMFLOAT4A colorf = XMFLOAT4A(1.0f, 1.0f, 1.0f, opacity);
	XMFLOAT4A colorf2 = XMFLOAT4A(1.0f, 1.0f, 1.0f, opacity + 0.2f);
	if(this->orientation == ORIENTATION_PORTRAIT)
	{
		colorf.w = 0.3f + 0.7f * opacity;
	}
	XMVECTOR colorv = XMLoadFloat4A(&colorf);
	XMVECTOR colorv2 = XMLoadFloat4A(&colorf2);
	

	Color cwhite(1.0f, 1.0f, 1.0f, 1.0f);
	Color cblack(1.0f, 1.0f, 1.0f, 1.0);
	Color cred(1.0f, 0.0f, 0.0f, 1.0f);


	//start drawing
	this->dxSpriteBatch->Begin(this->outputTransform);

	//===draw virtual controller
	if(this->orientation != ORIENTATION_PORTRAIT)
	{
		// Landscape
		Engine::Rectangle buttonsRect (this->buttonsRectangle.left, this->buttonsRectangle.top, this->buttonsRectangle.right - this->buttonsRectangle.left, this->buttonsRectangle.bottom - this->buttonsRectangle.top);

		ComPtr<ID3D11Texture2D> tex;
		this->buttonsLandscapeResource.As(&tex);
		this->dxSpriteBatch->Draw(buttonsRect, this->buttonsLandscapeSRV.Get(), tex.Get(), cblack);
	}else
	{
		// Portrait
		Engine::Rectangle buttonsRect (this->buttonsRectangle.left, this->buttonsRectangle.top, this->buttonsRectangle.right - this->buttonsRectangle.left, this->buttonsRectangle.bottom - this->buttonsRectangle.top);

		ComPtr<ID3D11Texture2D> tex;
		this->buttonsPortraitResource.As(&tex);
		if (cstate->APressed || cstate->BPressed)
			this->dxSpriteBatch->Draw(buttonsRect, this->buttonsPortraitSRV.Get(), tex.Get(), cred);
		else
			this->dxSpriteBatch->Draw(buttonsRect, this->buttonsPortraitSRV.Get(), tex.Get(), cblack);
	}

	if(settings->DPadStyle == 0 || settings->DPadStyle == 1)
	{
		Engine::Rectangle crossRect (this->crossRectangle.left, this->crossRectangle.top, this->crossRectangle.right - this->crossRectangle.left, this->crossRectangle.bottom - this->crossRectangle.top);

		ComPtr<ID3D11Texture2D> tex;
		this->crossResource.As(&tex);

		if (cstate->JoystickPressed)
			this->dxSpriteBatch->Draw(crossRect, this->crossSRV.Get(), tex.Get(), cred);
		else
			this->dxSpriteBatch->Draw(crossRect, this->crossSRV.Get(), tex.Get(), cblack);
	}else if(settings->DPadStyle == 2 || settings->DPadStyle == 3 )
	{
		RECT centerRect;
		RECT stickRect;
		this->controller->GetStickRectangle(&stickRect);
		this->controller->GetStickCenterRectangle(&centerRect);

		Engine::Rectangle stickRectE (stickRect.left, stickRect.top, stickRect.right - stickRect.left, stickRect.bottom - stickRect.top);
		Engine::Rectangle stickRectCenterE (centerRect.left, centerRect.top, centerRect.right - centerRect.left, centerRect.bottom - centerRect.top);

		ComPtr<ID3D11Texture2D> tex;
		this->stickResource.As(&tex);
		ComPtr<ID3D11Texture2D> tex2;
		this->stickCenterResource.As(&tex2);
		if (cstate->JoystickPressed)
		{
			this->dxSpriteBatch->Draw(stickRectCenterE, this->stickCenterSRV.Get(), tex2.Get(), cred);
			this->dxSpriteBatch->Draw(stickRectE, this->stickSRV.Get(), tex.Get(), cred);
		}
		else
		{
			this->dxSpriteBatch->Draw(stickRectCenterE, this->stickCenterSRV.Get(), tex2.Get(), cblack);
			this->dxSpriteBatch->Draw(stickRectE, this->stickSRV.Get(), tex.Get(), cblack);
		}
		
	}

	//start, select button
	Engine::Rectangle startSelectRectE (startSelectRectangle.left, startSelectRectangle.top, startSelectRectangle.right - startSelectRectangle.left, startSelectRectangle.bottom - startSelectRectangle.top);

	ComPtr<ID3D11Texture2D> texSS;
	this->startSelectResource.As(&texSS);

	if (cstate->StartPressed || cstate->SelectPressed)
		this->dxSpriteBatch->Draw(startSelectRectE, this->startSelectSRV.Get(), texSS.Get(), cred);
	else
		this->dxSpriteBatch->Draw(startSelectRectE, this->startSelectSRV.Get(), texSS.Get(), cblack);

	//L R buttons
	Engine::Rectangle lRectE (lRectangle.left, lRectangle.top, lRectangle.right - lRectangle.left, lRectangle.bottom - lRectangle.top);
	Engine::Rectangle rRectE (rRectangle.left, rRectangle.top, rRectangle.right - rRectangle.left, rRectangle.bottom - rRectangle.top);

	ComPtr<ID3D11Texture2D> tex;
	this->lButtonResource.As(&tex);
	ComPtr<ID3D11Texture2D> tex2;
	this->rButtonResource.As(&tex2);

	if (cstate->LPressed)
		this->dxSpriteBatch->Draw(lRectE, this->lButtonSRV.Get(), tex.Get(), cred);
	else
		this->dxSpriteBatch->Draw(lRectE, this->lButtonSRV.Get(), tex.Get(), cblack);

	if (cstate->RPressed)
		this->dxSpriteBatch->Draw(rRectE, this->rButtonSRV.Get(), tex2.Get(), cred);
	else
		this->dxSpriteBatch->Draw(rRectE, this->rButtonSRV.Get(), tex2.Get(), cblack);

	//end 
	this->dxSpriteBatch->End();

	frames++;
}

void *CPositionRenderer::MapBuffer(int index, size_t *rowPitch)
{
	D3D11_MAPPED_SUBRESOURCE map;
	ZeroMemory(&map, sizeof(D3D11_MAPPED_SUBRESOURCE));

	DX::ThrowIfFailed(
		this->m_d3dContext->Map(this->buffers[index].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map)
		);

	*rowPitch = map.RowPitch;
	return map.pData;
}
