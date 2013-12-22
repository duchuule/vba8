#pragma once

#include <System.h>
#include <ppltasks.h>

using namespace DirectX;
using namespace concurrency;
using namespace Windows::Storage;

//#define EXT_WIDTH (MAX_SNES_WIDTH + 4)
//#define EXT_PITCH (EXT_WIDTH * 2)
//#define EXT_HEIGHT (MAX_SNES_HEIGHT + 4)
//#define EXT_OFFSET (EXT_PITCH * 2 + 2 * 2)

extern CRITICAL_SECTION pauseSync;

namespace Emulator
{
	struct ROMData
	{
		unsigned char *ROM;
		size_t Length;
	};

	class EmulatorGame
	{
	public:
		static EmulatorGame *GetInstance(void);
		static EmulatedSystem emulator;

		EmulatorGame(void);
		~EmulatorGame(void);

		void Update(void *buffer, size_t rowPitch);

		void SetButtonState(int button, bool pressed);

		bool IsROMLoaded(void);
		bool IsPaused(void);
		void Pause(void);
		void Unpause(void);
		task<void> StopEmulationAsync(void);

		void Resume(void);
		void Suspend(void);

		void Start(void);
		void InitThread(void);
		void DeInitThread(void);
		void StopThread(void);
		void StartThread(void);
	private:
		static EmulatorGame instance;
		static bool initialized;

		Windows::Foundation::IAsyncAction ^threadAction;
		CRITICAL_SECTION cs;
		HANDLE swapEvent;
		HANDLE updateEvent;
		HANDLE sleepEvent;
		bool stopThread;
		
		BYTE *gfxbuffer;

		void Initialize(void);
		void UpdateAsync(void);
		void InitEmulator(void);

		void InitSound(void);
		void DeInitSound(void);
	};
}