#pragma once

#include <ppltasks.h>
#include "Direct3DBase.h"
#include "Emulator.h"
#include "defines.h"
#include "VirtualController.h"
#include "EmulatorSettings.h"
#include <collection.h>
#include "DXSpriteBatch.h"

using namespace Engine;
using namespace Emulator;
using namespace DirectX;
using namespace concurrency;
using namespace Windows::Storage;
using namespace Windows::UI::Input;
using namespace Microsoft::WRL;
using namespace PhoneDirect3DXamlAppComponent;

// This class renders a simple spinning cube.
ref class Renderer abstract: public Direct3DBase 
{
public:
	virtual ~Renderer(void);

internal:
	Renderer();
	

	// Direct3DBase methods.
	virtual void CreateDeviceResources() override;
	virtual void CreateWindowSizeDependentResources() override;
	virtual void Render() override;
	virtual void UpdateForWindowSizeChange(float width, float height) override;
	
	// Method for updating time-dependent objects.
	void Update(float timeTotal, float timeDelta);
	void ChangeOrientation(int orientation);



	size_t pitch;
	uint8 *backbufferPtr;
	float elapsedTime;
	bool autosaving;
	HANDLE waitEvent;
	int frames;


	RECT buttonsRectangle;
	RECT crossRectangle;
	RECT startSelectRectangle;
	RECT lRectangle;
	RECT rRectangle;

	int									orientation;
	int									format;
	int									frontbuffer;
	int									width, height;

	DXSpriteBatch						*dxSpriteBatch;
	EmulatorGame						*emulator;
	ComPtr<ID3D11Texture2D>				buffers[2];
	ComPtr<ID3D11ShaderResourceView>	bufferSRVs[2];
	ComPtr<ID3D11BlendState>			alphablend;

	ComPtr<ID3D11Resource>				stickCenterResource;
	ComPtr<ID3D11ShaderResourceView>	stickCenterSRV;
	ComPtr<ID3D11Resource>				stickResource;
	ComPtr<ID3D11ShaderResourceView>	stickSRV;
	ComPtr<ID3D11Resource>				crossResource;
	ComPtr<ID3D11ShaderResourceView>	crossSRV;
	ComPtr<ID3D11Resource>				buttonsLandscapeResource;
	ComPtr<ID3D11ShaderResourceView>	buttonsLandscapeSRV;
	ComPtr<ID3D11Resource>				buttonsPortraitResource;
	ComPtr<ID3D11ShaderResourceView>	buttonsPortraitSRV;
	ComPtr<ID3D11Resource>				startSelectResource;
	ComPtr<ID3D11ShaderResourceView>	startSelectSRV;
	ComPtr<ID3D11Resource>				lButtonResource;
	ComPtr<ID3D11ShaderResourceView>	lButtonSRV;
	ComPtr<ID3D11Resource>				rButtonResource;
	ComPtr<ID3D11ShaderResourceView>	rButtonSRV;
	EmulatorSettings					^settings;
	XMMATRIX							outputTransform;
	
	void CreateTransformMatrix(void);
};
