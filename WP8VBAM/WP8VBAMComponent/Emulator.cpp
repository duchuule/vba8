#include "pch.h"
#include "Emulator.h"
#include "EmulatorFileHandler.h"
#include "EmulatorSettings.h"
#include <ppltasks.h>
#include <windows.h>
#include <GBA.h>
#include <Util.h>
#include <SoundDriver.h>
#include <gbGlobals.h>

/* Link
---------------------*/
#include "GBALink.h"
/* ---------------- */


using namespace concurrency;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace PhoneDirect3DXamlAppComponent;

extern int emulating;
extern void ContinueEmulation(void);

//extern CRITICAL_SECTION swapCS;
//extern bool csInit;

extern SoundDriver *soundDriver;
extern long  soundSampleRate;

extern void soundShutdown(void);
extern bool soundInit(void);
extern void soundSetVolume(float);

CRITICAL_SECTION pauseSync;

namespace Emulator
{
	// Singleton
	EmulatorGame EmulatorGame::instance;
	bool EmulatorGame::initialized = false;

	EmulatedSystem EmulatorGame::emulator = GBASystem;


	EmulatorGame *EmulatorGame::GetInstance(void)
	{
		if(!initialized)
		{
			EmulatorGame::instance.Initialize();
			initialized = true;
		}
		return &EmulatorGame::instance;
	}


	EmulatorGame::EmulatorGame(void)
		: stopThread(false), gfxbuffer(nullptr), threadAction(nullptr), saveSRAMWhenStop(true)
	{ 
	}

	EmulatorGame::~EmulatorGame(void)
	{
		// Terminate thread
		this->DeInitThread();

		// Close handles
		CloseHandle(this->sleepEvent);
		CloseHandle(this->updateEvent);
		CloseHandle(this->swapEvent);
		CloseHandle(this->endEvent);
		DeleteCriticalSection(&this->cs);
		DeleteCriticalSection(&pauseSync);

		if(this->gfxbuffer)
		{
			delete [] this->gfxbuffer;
			this->gfxbuffer = nullptr;
		}

		/*if(csInit)
		{
			DeleteCriticalSection(&swapCS);
			csInit = false;
		}*/
	}

	void EmulatorGame::Initialize(void)
	{
		this->InitEmulator();

		this->swapEvent = CreateEventEx(NULL, NULL, CREATE_EVENT_INITIAL_SET, EVENT_ALL_ACCESS);
		this->updateEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);
		this->endEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);
		this->sleepEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);

		InitializeCriticalSectionEx(&this->cs, 0, 0);
		InitializeCriticalSectionEx(&pauseSync, NULL, NULL);

		// Create and start Thread
		this->InitThread();
	}

	void EmulatorGame::InitEmulator(void)
	{
		systemColorDepth = 32;
		systemRedShift = 19;
		systemBlueShift = 3;
		systemGreenShift = 11;

		utilUpdateSystemColorMaps();
	}

	void EmulatorGame::InitSound(void)
	{
		soundInit();

		/*if(soundDriver)
		{
			soundDriver->init(soundSampleRate);
			if(EmulatorSettings::Current->SoundEnabled)
			{
				soundSetVolume(1.0f);
			}else
			{
				soundSetVolume(0.0f);
			}
		}*/
	}

	void EmulatorGame::DeInitSound(void)
	{
		soundShutdown();

		/*if(soundDriver)
		{
			soundDriver->close();
		}*/
	}

	void EmulatorGame::InitThread(void)
	{
		EnterCriticalSection(&this->cs);

		if(this->threadAction)
		{
			LeaveCriticalSection(&this->cs);
			return;
		}

		this->stopThread = false;
		threadAction = ThreadPool::RunAsync(ref new WorkItemHandler([this](IAsyncAction ^action)
		{
			this->UpdateAsync();
		}), WorkItemPriority::High, WorkItemOptions::None);

		LeaveCriticalSection(&this->cs);
	}

	void EmulatorGame::DeInitThread(void)
	{
		EnterCriticalSection(&this->cs);

		if(this->stopThread)
		{
			LeaveCriticalSection(&this->cs);
			return;
		}

		this->stopThread = true;

		ContinueEmulation();

		//SetEvent(this->updateEvent);
		// Wait for thread termination
		WaitForSingleObjectEx(this->endEvent, INFINITE, false);

		this->threadAction = nullptr;

		LeaveCriticalSection(&this->cs);
	}

	void EmulatorGame::Resume(void)
	{		
		if(!this->threadAction)
		{
			ResetEvent(this->swapEvent);
			ResetEvent(this->updateEvent);
		}

		

		this->InitSound();		
		this->InitThread();

		//this->Start();
	}

	void EmulatorGame::Suspend(void)
	{
		this->DeInitThread();
		this->DeInitSound();
	}

	void EmulatorGame::Start(void)
	{
		SetEvent(this->updateEvent);
	}

	void EmulatorGame::StopThread(void)
	{
		this->stopThread = true;
	}

	void EmulatorGame::StartThread(void)
	{
		this->stopThread = false;
	}


	bool EmulatorGame::IsPaused(void)
	{
		return !emulating;
	}

	void EmulatorGame::Pause(void)
	{
	/*	ContinueEmulation();*/
		if(emulating)
		{
			EnterCriticalSection(&pauseSync);
			emulating = FALSE;
		}
		//LeaveCriticalSection(&this->pauseSync);
	}

	void EmulatorGame::Unpause(void)
	{
		//EnterCriticalSection(&this->pauseSync);
		

		if(!emulating)
		{
			emulating = TRUE;
			LeaveCriticalSection(&pauseSync);
		}
	}

	bool EmulatorGame::IsROMLoaded(void)
	{
		return ROMFile && ROMFolder;
	}

	task<void> EmulatorGame::StopEmulationAsync(void)
	{
		return create_task([this]()
		{
			if(this->IsROMLoaded())
			{
				this->Pause();

				//SaveSRAMAsync().wait();
				//int oldstate = SavestateSlot;
				//SavestateSlot = AUTOSAVE_SLOT;
				//SaveStateAsync().wait();
				//SavestateSlot = oldstate;
				//this->Pause();


				this->InitSound();
				//Memory.ClearSRAM();
				ROMFile = nullptr;
				ROMFolder = nullptr;
			}
		});
	}

	void EmulatorGame::SetButtonState(int button, bool pressed)
	{
	}

	void EmulatorGame::UpdateAsync(void)
	{
		WaitForSingleObjectEx(this->updateEvent, INFINITE, false);

		gbColorOption = EmulatorSettings::Current->UseGameboyColor;
		while(!stopThread)
		{
			

			//if(emulating)
			//{

				/*for (int i = 0; i < 2; i++)
				{
				emulator.emuMain(emulator.emuCount);
				}*/
				/*if(emulating)
				{*/
			emulator.emuMain(emulator.emuCount);
				/*}else
				{
					WaitForSingleObjectEx(this->sleepEvent, 0, false);
				}*/
#ifndef NO_LINK
			if (GetLinkMode() != LINK_DISCONNECTED)
				CheckLinkConnection();
#endif
			
		/*	}*/
			/*LeaveCriticalSection(&pauseSync);
			EnterCriticalSection(&pauseSync);*/
			/*SetEvent(this->swapEvent);
			WaitForSingleObjectEx(this->updateEvent, INFINITE, false);*/
		}

		// Signal for terminated thread
		SetEvent(this->endEvent);  //this is only called once when the user exists the game.


		OutputDebugStringW(L"THREAD ENDED.\n");
	}

	void EmulatorGame::Update(void *buffer, size_t rowPitch)
	{
		/*if(emulating)
		{
			WaitForSingleObjectEx(this->swapEvent, INFINITE, false);

			pix = (u8 *) buffer;

			SetEvent(this->updateEvent);
		}*/

		//if(!Settings.StopEmulation && this->IsROMLoaded())
		//{
		//	Settings.Mute = !EmulatorSettings::Current->SoundEnabled;

		//	WaitForSingleObjectEx(this->swapEvent, INFINITE, false);

		//	// Swap buffers
			//GFX.Screen = (uint16 *) buffer;
			//GFX.Pitch = rowPitch;

		//	SetEvent(this->updateEvent);
		//}
	}
}