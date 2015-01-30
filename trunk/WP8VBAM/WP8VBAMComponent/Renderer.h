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

#define CROSS_TEXTURE_FILE_NAME						L"Assets/Direct3D/pad_cross.dds"
#define START_TEXTURE_FILE_NAME						L"Assets/Direct3D/pad_start.dds"
#define SELECT_TEXTURE_FILE_NAME					L"Assets/Direct3D/pad_select.dds"
#define TURBO_TEXTURE_FILE_NAME						L"Assets/Direct3D/pad_turbo_button.dds"
#define COMBO_TEXTURE_FILE_NAME						L"Assets/Direct3D/pad_combo_button.dds"
#define A_TEXTURE_FILE_NAME							L"Assets/Direct3D/pad_a_button.dds"
#define B_TEXTURE_FILE_NAME							L"Assets/Direct3D/pad_b_button.dds"
#define L_TEXTURE_FILE_NAME							L"Assets/Direct3D/pad_l_button.dds"
#define R_TEXTURE_FILE_NAME							L"Assets/Direct3D/pad_r_button.dds"
#define STICK_TEXTURE_FILE_NAME						L"Assets/Direct3D/thumbstick.dds"
#define STICK_CENTER_TEXTURE_FILE_NAME				L"Assets/Direct3D/thumbstickcenter.dds"

#define CROSS_COLOR_TEXTURE_FILE_NAME				L"Assets/Direct3D/pad_cross_color.dds"
#define START_COLOR_TEXTURE_FILE_NAME				L"Assets/Direct3D/pad_start_color.dds"
#define SELECT_COLOR_TEXTURE_FILE_NAME				L"Assets/Direct3D/pad_select_color.dds"
#define TURBO_COLOR_TEXTURE_FILE_NAME				L"Assets/Direct3D/pad_turbo_button_color.dds"
#define COMBO_COLOR_TEXTURE_FILE_NAME				L"Assets/Direct3D/pad_combo_button_color.dds"
#define A_COLOR_TEXTURE_FILE_NAME					L"Assets/Direct3D/pad_a_button_color.dds"
#define B_COLOR_TEXTURE_FILE_NAME					L"Assets/Direct3D/pad_b_button_color.dds"
#define L_COLOR_TEXTURE_FILE_NAME					L"Assets/Direct3D/pad_l_button_color.dds"
#define R_COLOR_TEXTURE_FILE_NAME					L"Assets/Direct3D/pad_r_button_color.dds"
#define STICK_COLOR_TEXTURE_FILE_NAME				L"Assets/Direct3D/thumbstick_color.dds"

#define CROSS_GBASP_TEXTURE_FILE_NAME				L"Assets/Direct3D/pad_cross_gbasp.dds"
#define START_GBASP_TEXTURE_FILE_NAME				L"Assets/Direct3D/pad_start_gbasp.dds"
#define SELECT_GBASP_TEXTURE_FILE_NAME				L"Assets/Direct3D/pad_select_gbasp.dds"
#define TURBO_GBASP_TEXTURE_FILE_NAME				L"Assets/Direct3D/pad_turbo_button_gbasp.dds"
#define COMBO_GBASP_TEXTURE_FILE_NAME				L"Assets/Direct3D/pad_combo_button_gbasp.dds"
#define A_GBASP_TEXTURE_FILE_NAME					L"Assets/Direct3D/pad_a_button_gbasp.dds"
#define B_GBASP_TEXTURE_FILE_NAME					L"Assets/Direct3D/pad_b_button_gbasp.dds"
#define L_GBASP_TEXTURE_FILE_NAME					L"Assets/Direct3D/pad_l_button_gbasp.dds"
#define R_GBASP_TEXTURE_FILE_NAME					L"Assets/Direct3D/pad_r_button_gbasp.dds"
#define STICK_GBASP_TEXTURE_FILE_NAME				L"Assets/Direct3D/thumbstick_gbasp.dds"

#define DIVIDER_TEXTURE_FILE_NAME					L"Assets/Direct3D/divider.dds"
#define RESUME_TEXTURE_FILE_NAME					L"Assets/Direct3D/resumetext.dds"

#define AUTOSAVE_INTERVAL			60.0f

// This class renders a simple spinning cube.
ref class Renderer abstract: public Direct3DBase 
{
public:
	virtual ~Renderer(void);

protected:
	void DrawController(void);

internal:
	Renderer();
	

	// Direct3DBase methods.
	virtual void CreateDeviceResources() override;
	virtual void CreateWindowSizeDependentResources() override;
	virtual void Render() override;
	virtual void UpdateForWindowSizeChange(float width, float height) override;
	
	// Method for updating time-dependent objects.
	virtual void Update(float timeTotal, float timeDelta);


	size_t pitch;
	uint8 *backbufferPtr;
	float elapsedTime;
	bool autosaving;
	HANDLE waitEvent;
	int frames;


	RECT aRectangle;
	RECT bRectangle;
	RECT crossRectangle;
	RECT startRectangle;
	RECT selectRectangle;
	RECT turboRectangle;
	RECT comboRectangle;
	RECT lRectangle;
	RECT rRectangle;
	RECT stickRect;
	RECT centerRect;

	Color joystick_color;
	Color joystick_center_color;
	Color l_color;
	Color r_color;
	Color select_color;
	Color start_color;
	Color turbo_color;
	Color combo_color;
	Color a_color;
	Color b_color;
	Color resume_text_color;
	int pad_to_draw;
	bool should_draw_LR;


	int									orientation;
	int									format;
	int									frontbuffer;
	int									width, height;
	bool								should_show_resume_text;

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
	ComPtr<ID3D11Resource>				aResource;
	ComPtr<ID3D11ShaderResourceView>	aSRV;
	ComPtr<ID3D11Resource>				bResource;
	ComPtr<ID3D11ShaderResourceView>	bSRV;
	ComPtr<ID3D11Resource>				startResource;
	ComPtr<ID3D11ShaderResourceView>	startSRV;
	ComPtr<ID3D11Resource>				selectResource;
	ComPtr<ID3D11ShaderResourceView>	selectSRV;
	ComPtr<ID3D11Resource>				turboResource;
	ComPtr<ID3D11ShaderResourceView>	turboSRV;
	ComPtr<ID3D11Resource>				comboResource;
	ComPtr<ID3D11ShaderResourceView>	comboSRV;
	ComPtr<ID3D11Resource>				lButtonResource;
	ComPtr<ID3D11ShaderResourceView>	lButtonSRV;
	ComPtr<ID3D11Resource>				rButtonResource;
	ComPtr<ID3D11ShaderResourceView>	rButtonSRV;
	ComPtr<ID3D11Resource>				dividerResource;
	ComPtr<ID3D11ShaderResourceView>	dividerSRV;
	ComPtr<ID3D11Resource>				resumeTextResource;
	ComPtr<ID3D11ShaderResourceView>	resumeTextSRV;
	EmulatorSettings					^settings;
	XMMATRIX							outputTransform;
	
	void CreateTransformMatrix(void);
};
