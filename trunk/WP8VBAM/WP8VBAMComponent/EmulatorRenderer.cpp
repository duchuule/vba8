#include "pch.h"
#include "EmulatorRenderer.h"
#include "EmulatorFileHandler.h"
#include "Vector4.h"
#include "TextureLoader.h"
#include "WP8VBAMComponent.h"
#include <math.h>

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Windows::Graphics::Display;


HANDLE swapEvent = NULL;
HANDLE updateEvent = NULL;
//CRITICAL_SECTION swapCS;
//bool csInit = false;

bool lastSkipped = false;

namespace Emulator
{
	extern bool gbaROMLoaded;
}

void ContinueEmulation(void)
{
	if(swapEvent && updateEvent)
	{
		ResetEvent(swapEvent);
		SetEvent(updateEvent);
	}
}

extern u8 *pix;
size_t gbaPitch;
int turboSkip = 5;

int framesNotRendered = 0;

//u8 tmpBuf[241 * 162 * 4];

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

EmulatorRenderer::EmulatorRenderer()
{ 
	emulator = EmulatorGame::GetInstance();
	frontbuffer = 0;
	controller = nullptr;
	autosaving = false;
	elapsedTime = 0.0f;
	settings = EmulatorSettings::Current;
	frames = 0;
	should_show_resume_text = false;

	this->waitEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);

	swapEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);
	updateEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);

	/*if(!csInit)
	{
	InitializeCriticalSectionEx(&swapCS, NULL, NULL);
	csInit = true;
	}*/

}

EmulatorRenderer::~EmulatorRenderer(void)
{
	if(this->m_d3dContext)
	{
		this->m_d3dContext->Unmap(this->buffers[(this->frontbuffer + 1) % 2].Get(), 0);
	}

	CloseHandle(this->waitEvent);
	CloseHandle(swapEvent);
	CloseHandle(updateEvent);

	delete this->dxSpriteBatch;
	this->dxSpriteBatch = nullptr;
}

void EmulatorRenderer::CreateDeviceResources()
{
	Renderer::CreateDeviceResources();
	


	// Map backbuffer so it can be unmapped on first update
	int backbuffer = (this->frontbuffer + 1) % 2;
	this->backbufferPtr = (uint8 *) this->MapBuffer(backbuffer, &this->pitch);
	pix = this->backbufferPtr;
}

void EmulatorRenderer::CreateWindowSizeDependentResources()
{
	Direct3DBase::CreateWindowSizeDependentResources();
}

void EmulatorRenderer::UpdateForWindowSizeChange(float width, float height)
{
	Direct3DBase::UpdateForWindowSizeChange(width, height);

	float scale = ((int)DisplayProperties::ResolutionScale) / 100.0f;
	this->height = width * scale;
	this->width = height * scale;

	if(!this->dxSpriteBatch)
	{
		this->dxSpriteBatch = new DXSpriteBatch(this->m_d3dDevice.Get(), this->m_d3dContext.Get(), settings->UseLinearFilter, this->height, this->width);
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

void EmulatorRenderer::UpdateController(void)
{
	if(this->controller)
	{
		this->controller->UpdateFormat(this->format);
	}
}


void EmulatorRenderer::SetVirtualController(VirtualController *controller)
{
	this->controller = controller;
	this->controller->UpdateFormat(this->format);
	this->controller->SetOrientation(this->orientation);
}

void EmulatorRenderer::GetBackbufferData(uint8 **backbufferPtr, size_t *pitch, int *imageWidth, int *imageHeight)
{
	*backbufferPtr = this->backbufferPtr + this->pitch;
	*pitch = this->pitch;
	if(gbaROMLoaded)
	{
		*imageWidth = 240;
		*imageHeight = 160;
	}else
	{
		*imageWidth = 160;
		*imageHeight = 144;
	}
}

void EmulatorRenderer::ChangeOrientation(int orientation)
{
	this->orientation = orientation;
	if(this->controller)
	{
		this->controller->SetOrientation(this->orientation);
	}
	this->CreateTransformMatrix();
}


void EmulatorRenderer::Update(float timeTotal, float timeDelta)
{
	if(!emulator->IsPaused())
	{
		this->elapsedTime += timeDelta;

		systemFrameSkip = settings->PowerFrameSkip;
		float targetFPS = 55.0f;
		if(/*settings->LowFrequencyModeMeasured && */settings->LowFrequencyMode)
		{
			systemFrameSkip = systemFrameSkip * 2 + 1;
			targetFPS = 28.0f;
		}
		if(settings->FrameSkip == -1 && settings->PowerFrameSkip == 0)
		{
			if(!lastSkipped && (timeDelta * 1.0f) > (1.0f / targetFPS))
			{
				int skip = (int)((timeDelta * 1.0f) / (1.0f / targetFPS));				
				systemFrameSkip += (skip < 2) ? skip : 2;
				//systemFrameSkip++;
				lastSkipped = true;
			}else
			{
				lastSkipped = false;
			}
		}else if(settings->FrameSkip >= 0)
		{
			systemFrameSkip += settings->FrameSkip;
		}
	}
	/*if(!settings->LowFrequencyModeMeasured)
	{
		if(this->elapsedTime >= 3.0f)
		{
			if(this->frames < 100)
			{
				settings->LowFrequencyMode = true;
			}else
			{
				settings->LowFrequencyMode = false;
			}
			settings->LowFrequencyModeMeasured = true;
		}
	}*/
	if(this->elapsedTime >= AUTOSAVE_INTERVAL)
	{
		if(!emulator->IsPaused())
		{
			this->elapsedTime -= AUTOSAVE_INTERVAL;
			autosaving = true;
			emulator->Pause();
			SaveSRAMAsync().then([this]()
			{
				if (settings->AutoSaveLoad)
				{
					int oldSlot = SavestateSlot;
					SavestateSlot = AUTOSAVE_SLOT;
					SaveStateAsync().wait();
					SavestateSlot = oldSlot;
					//Settings.Mute = !EmulatorSettings::Current->SoundEnabled;
				}
				emulator->Unpause();
				SetEvent(this->waitEvent);
			});
		}else
		{
			this->elapsedTime = AUTOSAVE_INTERVAL;
		}
	}

	float opacity = 1.0f;
	
	if(this->orientation != ORIENTATION_PORTRAIT || this->settings->VirtualControllerStyle == 2) //only opacity in landscape or when using simple button
		opacity = this->settings->ControllerOpacity / 100.0f;

	Color color(1.0f, 1.0f, 1.0f, opacity);
	Color color2(1.0f, 1.0f, 1.0f, opacity + 0.2f);
	joystick_color = color;
	joystick_center_color = color2;
	l_color = color;
	r_color = color;
	select_color = color;
	start_color = color;
	turbo_color = color;
	a_color = color;
	b_color = color;
	combo_color = color;
	
	
	float text_opacity = (sinf(timeTotal*2) + 1.0f) / 2.0f;
	resume_text_color = Color(1.0f, 0.0f, 0.0f, text_opacity);
	

	if(settings->DPadStyle == 0 || settings->DPadStyle == 1)
		pad_to_draw = 0;
	else if(settings->DPadStyle == 2 || (settings->DPadStyle == 3 && this->controller->StickFingerDown()))
		pad_to_draw = 1;
	else
		pad_to_draw = -1;



	should_draw_LR = gbaROMLoaded;

}

void EmulatorRenderer::Render()
{
	if(autosaving)
	{
		WaitForSingleObjectEx(this->waitEvent, INFINITE, false);
		autosaving = false;
	}

	

	m_d3dContext->OMSetRenderTargets(
		1,
		m_renderTargetView.GetAddressOf(),
		m_depthStencilView.Get()
		);

	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.000f }; //black
	if (this->settings->VirtualControllerStyle != 2 && this->orientation == ORIENTATION_PORTRAIT)
	{
		bgcolor[0] = (float)this->settings->BgcolorR / 255;
		bgcolor[1] = (float)this->settings->BgcolorG / 255;
		bgcolor[2] = (float)this->settings->BgcolorB / 255;
	}


	m_d3dContext->ClearRenderTargetView(
		m_renderTargetView.Get(),
		bgcolor
		);

	m_d3dContext->ClearDepthStencilView(
		m_depthStencilView.Get(),
		D3D11_CLEAR_DEPTH,
		1.0f,
		0
		);

	if(!this->emulator->IsPaused())
	{
		if(framesNotRendered >= settings->PowerFrameSkip)
		{
			framesNotRendered = 0;

			WaitForSingleObjectEx(swapEvent, INFINITE, false);
			int backbuffer = this->frontbuffer;
			this->frontbuffer = (this->frontbuffer + 1) % 2;
			uint8 *buffer = (uint8 *) this->MapBuffer(backbuffer, &gbaPitch);
			this->backbufferPtr = buffer;
			this->pitch = gbaPitch;

			pix = buffer;

			this->m_d3dContext->Unmap(this->buffers[this->frontbuffer].Get(), 0);

			SetEvent(updateEvent);
		}else
		{
			framesNotRendered++;
		}
	}

	int height, width;
	RECT rect;
	RECT dividerRect = RECT();


	if(this->orientation != ORIENTATION_PORTRAIT)
	{
		height = this->height * (this->settings->ImageScaling / 100.0f);
		switch(settings->AspectRatio)
		{
		default:
		case AspectRatioMode::Original:
			if(gbaROMLoaded)
			{
				width = (int)(height * (240.0f / 160.0f));
			}else
			{
				width = (int)(height * (160.0f / 144.0f));
			}
			break;
		case AspectRatioMode::Stretch:
			width = this->width * (this->settings->ImageScaling / 100.0f);
			break;
		case AspectRatioMode::FourToThree:
			width = (int)(height * (4.0f / 3.0f));
			break;
		case AspectRatioMode::FiveToFour:
			width = (int)(height * (5.0f / 4.0f));
			break;
		case AspectRatioMode::One:
			width = height;
			break;
		}

		int leftOffset = (this->width - width) / 2;
		rect.left = leftOffset;
		rect.right = width + leftOffset;
		rect.top = 0;
		rect.bottom = height;


	}else
	{
		width = this->height;

		switch(settings->AspectRatio)
		{
		default:
		case AspectRatioMode::Original:
		case AspectRatioMode::Stretch:
			if(gbaROMLoaded)
			{
				height = (int)(width * (160.0f / 240.0f));
			}else
			{
				height = (int)(width * (144.0f / 160.0f));
			}
			break;
		case AspectRatioMode::FourToThree:
			height = (int)(width * (3.0f / 4.0f));
			break;
		case AspectRatioMode::FiveToFour:
			height = (int)(width * (4.0f / 5.0f));
			break;
		case AspectRatioMode::One:
			height = (int)width;
			break;
		}


		rect.left = 0;
		rect.right = width;
		rect.top = 0;
		rect.bottom = height;

		dividerRect.left = 0;
		dividerRect.right = width;
		dividerRect.top = height;
		float scale = ((int)DisplayProperties::ResolutionScale) / 100.0f;
		dividerRect.bottom = height + 4 * scale;
	}

	RECT source;
	if(gbaROMLoaded)
	{
		source.left = 0;
		source.right = 240;
		source.top = 2;
		source.bottom = 161;
	}else
	{
		source.left = 0;
		source.right = 160;
		source.top = 2;
		source.bottom = 144;
	}



	float opacity = this->settings->ControllerOpacity / 100.0f;
	XMFLOAT4A colorf = XMFLOAT4A(1.0f, 1.0f, 1.0f, opacity);
	XMFLOAT4A colorf2 = XMFLOAT4A(1.0f, 1.0f, 1.0f, opacity + 0.2f);
	if(this->orientation == ORIENTATION_PORTRAIT)
	{
		colorf.w = 0.3f + 0.7f * opacity;
	}
	XMVECTOR colorv = XMLoadFloat4A(&colorf);
	XMVECTOR colorv2 = XMLoadFloat4A(&colorf2);
	

	// Render last frame to screen
	Color white(1.0f, 1.0f, 1.0f, 1.0f);
	Color red(1.0f, 0.0f, 0.0f, 1.0f);
	Color color(1.0f, 1.0f, 1.0f, opacity);
	Color color2(1.0f, 1.0f, 1.0f, opacity + 0.2f);
	Color dividerColor(86.0f/255, 105.0f/255, 108.0f/255, 1.0f);

	this->dxSpriteBatch->Begin(this->outputTransform);
	

	Engine::Rectangle sourceRect(source.left, source.top, source.right - source.left, source.bottom - source.top);
	Engine::Rectangle targetRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);

	this->dxSpriteBatch->Draw(targetRect,  &sourceRect, this->bufferSRVs[this->frontbuffer].Get(), this->buffers[this->frontbuffer].Get(), white);

	//display resume text if paused
	if(should_show_resume_text)
	{
		int textWidth = 0.5*width;
		int textHeight = textWidth / 480.0f * 80.0f;

		Engine::Rectangle resumeTextRect ((width - textWidth) / 2, (height - textHeight) / 2, textWidth, textHeight);

		ComPtr<ID3D11Texture2D> tex;
		this->resumeTextResource.As(&tex);
		this->dxSpriteBatch->Draw(resumeTextRect, this->resumeTextSRV.Get(), tex.Get(), resume_text_color);

	}

	//draw divider 
	if(this->orientation == ORIENTATION_PORTRAIT && this->settings->VirtualControllerStyle != 2)
	{
		ComPtr<ID3D11Texture2D> tex;
		this->dividerResource.As(&tex);

		Engine::Rectangle targetRect(dividerRect.left, dividerRect.top, dividerRect.right - dividerRect.left, dividerRect.bottom - dividerRect.top);
		this->dxSpriteBatch->Draw(targetRect,  this->dividerSRV.Get(), tex.Get(), dividerColor);
	}
	//===draw virtual controller if moga controller is not loaded
	using namespace Moga::Windows::Phone;
	Moga::Windows::Phone::ControllerManager^ ctrl = Direct3DBackground::getController();
	if (!(EmulatorSettings::Current->UseMogaController && ctrl != nullptr && ctrl->GetState(Moga::Windows::Phone::ControllerState::Connection) == ControllerResult::Connected))
	{
		this->controller->GetARectangle(&aRectangle);
		this->controller->GetBRectangle(&bRectangle);
		this->controller->GetCrossRectangle(&crossRectangle);
		this->controller->GetStartRectangle(&startRectangle);
		this->controller->GetSelectRectangle(&selectRectangle);
		this->controller->GetTurboRectangle(&turboRectangle);
		this->controller->GetComboRectangle(&comboRectangle);
		this->controller->GetLRectangle(&lRectangle);
		this->controller->GetRRectangle(&rRectangle);
		this->controller->GetStickRectangle(&stickRect);
		this->controller->GetStickCenterRectangle(&centerRect);

		DrawController();
	}
	this->dxSpriteBatch->End();

	frames++;
}

void *EmulatorRenderer::MapBuffer(int index, size_t *rowPitch)
{
	D3D11_MAPPED_SUBRESOURCE map;
	ZeroMemory(&map, sizeof(D3D11_MAPPED_SUBRESOURCE));

	DX::ThrowIfFailed(
		this->m_d3dContext->Map(this->buffers[index].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map)
		);

	*rowPitch = map.RowPitch;
	return map.pData;
}

void systemDrawScreen() 
{ 
	LeaveCriticalSection(&pauseSync);

	SetEvent(swapEvent);

	WaitForSingleObjectEx(updateEvent, INFINITE, false);

	EnterCriticalSection(&pauseSync);
}