#include "pch.h"
#include "LinkContentProvider.h"

using namespace PhoneDirect3DXamlAppComponent;

LinkDirect3DContentProvider::LinkDirect3DContentProvider(LinkDirect3DBackground^ controller) :
	m_controller(controller)
{
	m_controller->RequestAdditionalFrame += ref new RequestAdditionalFrameHandler([=] ()
		{
			if (m_host)
			{
				m_host->RequestAdditionalFrame();
			}
		});
}

// IDrawingSurfaceContentProviderNative interface
HRESULT LinkDirect3DContentProvider::Connect(_In_ IDrawingSurfaceRuntimeHostNative* host, _In_ ID3D11Device1* device)
{
	m_host = host;

	return m_controller->Connect(host, device);
}

void LinkDirect3DContentProvider::Disconnect()
{
	m_controller->Disconnect();
	m_host = nullptr;
}

HRESULT LinkDirect3DContentProvider::PrepareResources(_In_ const LARGE_INTEGER* presentTargetTime, _Inout_ DrawingSurfaceSizeF* desiredRenderTargetSize)
{
	return m_controller->PrepareResources(presentTargetTime, desiredRenderTargetSize);
}

HRESULT LinkDirect3DContentProvider::Draw(_In_ ID3D11Device1* device, _In_ ID3D11DeviceContext1* context, _In_ ID3D11RenderTargetView* renderTargetView)
{
	return m_controller->Draw(device, context, renderTargetView);
}