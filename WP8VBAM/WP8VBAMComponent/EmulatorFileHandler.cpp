#include "pch.h"
#include <string>
#include <sstream>
#include <memory>
#include "EmulatorFileHandler.h"
#include <System.h>
#include <NLS.h>
#include <Port.h>
#include <GBA.h>
#include <gb.h>
#include <Globals.h>
#include <RTC.h>
#include <robuffer.h>
#include "EmulatorSettings.h"
#include "EmulatorRenderer.h"
#include <fstream>
#include <Util.h>
#include <Gb_Apu.h>
#include <Sound.h>
#include <gbSound.h>
#include <gbMemory.h>
#include <gbCheats.h>
#include <Cheats.h>

using namespace PhoneDirect3DXamlAppComponent;
using namespace Platform;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage::FileProperties;
using namespace std;

#define SAVE_FOLDER	"saves"

extern bool cheatsEnabled;
extern int gbaSaveType;
extern int romSize;
extern int emulating;

// Extern functions used for read and save state
extern bool soundInit();
extern void reset_apu();
extern void write_SGCNT0_H( int data );
extern void apply_muting();
extern void CPUUpdateWindow0();
extern void CPUUpdateWindow1();
extern void sramWrite(u32, u8);
extern void CPUReadHelper(void);

extern int gbBattery;
extern int gbRomType;
extern u8 *gbRam;
extern int gbRamSizeMask;
extern u8 *gbMemoryMap[16];
extern mapperMBC3 gbDataMBC3;
extern mapperTAMA5 gbDataTAMA5;
extern u8 *gbMemory;
extern u8 *gbTAMA5ram;
extern int gbTAMA5ramSize;
extern int gbRamSize;

namespace Emulator
{
	int ROMSize = 0;
	int ROMSize2 = 0;
	StorageFile ^ROMFile = nullptr;
	StorageFile ^ROMFile2 = nullptr;
	StorageFolder ^ROMFolder = nullptr;
	StorageFolder ^ROMFolder2 = nullptr;
	int SavestateSlot = 0;
	int LoadstateSlot = 0;
	int SavestateSlot2 = 0;
	int LoadstateSlot2 = 0;
	bool gbaROMLoaded = true; //false if gb ROM, true if gba ROM 
	bool gbaROMLoaded2 = true; //false if gb ROM, true if gba ROM 
	
	Windows::Foundation::Collections::IVector<PhoneDirect3DXamlAppComponent::CheatData ^> ^cheatsTmp = nullptr;

	Platform::Array<unsigned char> ^GetSnapshotBuffer(unsigned char *backbuffer, size_t pitch, int imageWidth, int imageHeight)
	{
		Platform::Array<unsigned char> ^buffer = ref new Platform::Array<unsigned char>(imageWidth * imageHeight * 4);		

		/*Microsoft::WRL::ComPtr<IBufferByteAccess> byteAccess;
		reinterpret_cast<IUnknown*>(buffer)->QueryInterface(IID_PPV_ARGS(&byteAccess));
		byte *buf;
		byteAccess->Buffer(&buf);
		uint16 *targetBuffer = (uint16 *) buf;*/
		int dstPitch = imageWidth * 4;
		for (int i = 0; i < imageHeight; i++)
		{
			for (int j = 0; j < imageWidth * 4; j+=4)
			{
				// red
				buffer[dstPitch * i + j] = *(backbuffer + pitch * i + j + 2);
				
				// green
				buffer[dstPitch * i + j + 1] = *(backbuffer + pitch * i + j + 1);
				
				// blue
				buffer[dstPitch * i + j + 2] = *(backbuffer + pitch * i + j + 0);
				
				// alpha
				buffer[dstPitch * i + j + 3] = 0xff;


				//*(targetBuffer + imageWidth * i + j) = *(backbuffer + (pitch / 2) * i + j) ;
			}
		}

		return buffer;
	}

	task<void> SaveStateAsync(void)
	{
		if(gbaROMLoaded)
		{
			return SaveGBAStateAsync();
		}else
		{
			return SaveGBStateAsync();
		}
	}

	task<void> SaveGBStateAsync(void)
	{
		EmulatorGame *emulator = EmulatorGame::GetInstance();
		return create_task([]()
		{
			if(!ROMFile || !ROMFolder)
			{
				throw ref new Exception(E_FAIL, "No ROM loaded.");
			}
			return ROMFolder->GetFolderAsync(SAVE_FOLDER);
		}).then([emulator](StorageFolder ^folder)
		{
			emulator->Pause();
		
			Platform::String ^folderpath = folder->Path;
			string folderPathStr(folderpath->Begin(), folderpath->End());

			StorageFile ^romFile = ROMFile;

			Platform::String ^tmp = ROMFile->Name;
			const wchar_t *end = tmp->End();
			while(*end != '.') end--;
			size_t diff = tmp->End() - end;

			wstring wRomName(ROMFile->Name->Begin(), ROMFile->Name->Length() - diff);
			string romName(wRomName.begin(), wRomName.end());

			stringstream tmpFileNameStream;
			tmpFileNameStream << folderPathStr << "\\";
			tmpFileNameStream << romName << SavestateSlot << ".sgm";
			string fileNameA = tmpFileNameStream.str();

			FILE *file;
			auto error = fopen_s(&file, fileNameA.c_str(), "wb");
			if(!file)
			{
#if _DEBUG
				stringstream ss;
				ss << "Unable to open file '";
				ss << fileNameA;
				ss << "' to store savestate (";
				ss << error;
				ss << ").";
				OutputDebugStringA(ss.str().c_str());
				return;
#endif
			}
			fclose(file);

			ofstream stream (fileNameA.c_str(), ios::binary);
			if(!stream.is_open())
			{
#if _DEBUG
				stringstream ss;
				ss << "Unable to open file '";
				ss << fileNameA;
				ss << "' to store savestate.";
				OutputDebugStringA(ss.str().c_str());
				return;
#endif
			}

			extern u8* gbRom;
			extern bool useBios;
			extern bool inBios;
			extern variable_desc gbSaveGameStruct[78];
			extern u16 IFF;
			extern int gbSgbMode;
			extern variable_desc gbSgbSaveStructV3[11];
			extern u8 *gbSgbBorder;
			extern u8 *gbSgbBorderChar;
			extern u8 gbSgbPacket[112];
			extern u16 gbSgbSCPPalette[2048];
			extern u8 gbSgbATF[360];
			extern u8 gbSgbATFList[16200];
			extern mapperMBC1 gbDataMBC1;
			extern mapperMBC2 gbDataMBC2;
			extern mapperMBC3 gbDataMBC3;
			extern mapperMBC5 gbDataMBC5;
			extern mapperHuC1 gbDataHuC1;
			extern mapperHuC3 gbDataHuC3;
			extern mapperTAMA5 gbDataTAMA5;
			extern u8 *gbTAMA5ram;
			extern int gbTAMA5ramSize;
			extern mapperMMM01 gbDataMMM01;
			extern u16 gbPalette[128];
			extern u8 *gbMemory;
			extern int gbRamSize;
			extern u8 *gbRam;
			extern int gbCgbMode;
			extern u8 *gbVram;
			extern u8 *gbWram;
			extern variable_desc gb_state[20];
			extern int gbCheatNumber;
			extern gbCheat gbCheatList[100];
			extern int gbLcdModeDelayed;
			extern int gbLcdTicksDelayed;
			extern int gbLcdLYIncrementTicksDelayed;
			extern u8 gbSpritesTicks[300];
			extern bool gbTimerModeChange;
			extern bool gbTimerOnChange;
			extern int gbHardware;
			extern bool gbBlackScreen;
			extern u8 oldRegister_WY;
			extern int gbWindowLine;
			extern int inUseRegister_WY;
			extern bool gbScreenOn;
			int marker = 0x12345678;

			int version = 12;
			stream.write(reinterpret_cast<const char *>(&version), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&gbRom[0x134]), 15);

			int ub = useBios;
			int ib = inBios;
			stream.write(reinterpret_cast<const char *>(&ub), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&ib), sizeof(int));

			int i = 0;
			for (; i < ARRAYSIZE(gbSaveGameStruct); i++)
			{
				if(gbSaveGameStruct[i].size > 0)
				{
					stream.write(reinterpret_cast<const char *>(gbSaveGameStruct[i].address), gbSaveGameStruct[i].size);
				}
			}

			stream.write(reinterpret_cast<const char *>(&IFF), sizeof(u16));

			if(gbSgbMode)
			{
				i = 0;
				for (; i < ARRAYSIZE(gbSgbSaveStructV3); i++)
				{
					if(gbSgbSaveStructV3[i].size > 0)
					{
						stream.write(reinterpret_cast<const char *>(gbSgbSaveStructV3[i].address), gbSgbSaveStructV3[i].size);
					}
				}

				stream.write(reinterpret_cast<const char *>(gbSgbBorder), 2048);
				stream.write(reinterpret_cast<const char *>(gbSgbBorderChar), 32*256);

				stream.write(reinterpret_cast<const char *>(gbSgbPacket), 16 * 7);

				stream.write(reinterpret_cast<const char *>(gbSgbSCPPalette), 4 * 512 * sizeof(u16));
				stream.write(reinterpret_cast<const char *>(gbSgbATF), 20 * 18);
				stream.write(reinterpret_cast<const char *>(gbSgbATFList), 45 * 20 * 18);
			}

			stream.write(reinterpret_cast<const char *>(&gbDataMBC1), sizeof(gbDataMBC1));
			stream.write(reinterpret_cast<const char *>(&gbDataMBC2), sizeof(gbDataMBC2));
			stream.write(reinterpret_cast<const char *>(&gbDataMBC3), sizeof(gbDataMBC3));
			stream.write(reinterpret_cast<const char *>(&gbDataMBC5), sizeof(gbDataMBC5));
			stream.write(reinterpret_cast<const char *>(&gbDataHuC1), sizeof(gbDataHuC1));
			stream.write(reinterpret_cast<const char *>(&gbDataHuC3), sizeof(gbDataHuC3));
			stream.write(reinterpret_cast<const char *>(&gbDataTAMA5), sizeof(gbDataTAMA5));
			if(gbTAMA5ram != NULL)
			{
				stream.write(reinterpret_cast<const char *>(gbTAMA5ram), gbTAMA5ramSize);
			}
			stream.write(reinterpret_cast<const char *>(&gbDataMMM01), sizeof(gbDataMMM01));

			stream.write(reinterpret_cast<const char *>(gbPalette), 128 * sizeof(u16));

			stream.write(reinterpret_cast<const char *>(&gbMemory[0x8000]), 0x8000);

			if(gbRamSize && gbRam)
			{
				stream.write(reinterpret_cast<const char *>(&gbRamSize), sizeof(int));
				stream.write(reinterpret_cast<const char *>(gbRam), gbRamSize);
			}

			if(gbCgbMode)
			{
				stream.write(reinterpret_cast<const char *>(gbVram), 0x4000);
				stream.write(reinterpret_cast<const char *>(gbWram), 0x8000);
			}

			// Sound
			gbSoundSaveGame2();
			i = 0;
			for (; i < ARRAYSIZE(gb_state); i++)
			{
				if(gb_state[i].size > 0)
				{
					stream.write(reinterpret_cast<const char *>(gb_state[i].address), gb_state[i].size);
				}
			}

			// Cheats
			stream.write(reinterpret_cast<const char *>(&gbCheatNumber), sizeof(int));
			if(gbCheatNumber > 0)
			{
				stream.write(reinterpret_cast<const char *>(&gbCheatList[0]), sizeof(gbCheat)*gbCheatNumber);
			}

			int spriteTicks = gbSpritesTicks[299];
			int timerModeChange = gbTimerModeChange;
			int timerOnChange = gbTimerOnChange;
			int blackScreen = gbBlackScreen;
			int oldRegister = oldRegister_WY;
			int screenOn = gbScreenOn;
			stream.write(reinterpret_cast<const char *>(&gbLcdModeDelayed), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&gbLcdTicksDelayed), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&gbLcdLYIncrementTicksDelayed), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&spriteTicks), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&timerModeChange), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&timerOnChange), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&gbHardware), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&blackScreen), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&oldRegister), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&gbWindowLine), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&inUseRegister_WY), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&screenOn), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&marker), sizeof(int));

			stream.flush();
			stream.close();
		}).then([emulator](task<void> t)
		{
			try
			{
				//emulator->Unpause();	
				t.get();
			}catch(Exception ^ex)
			{
#if _DEBUG
				wstring str(ex->Message->Begin(), ex->Message->End());
				OutputDebugStringW((L"Save state: " + str).c_str());
#endif
			}
		});
	}

	task<void> SaveGBAStateAsync(void)
	{
		EmulatorGame *emulator = EmulatorGame::GetInstance();
		return create_task([]()
		{
			if(!ROMFile || !ROMFolder)
			{
				throw ref new Exception(E_FAIL, "No ROM loaded.");
			}
			return ROMFolder->GetFolderAsync(SAVE_FOLDER);
		}).then([emulator](StorageFolder ^folder)
		{
			emulator->Pause();
		
			Platform::String ^folderpath = folder->Path;
			string folderPathStr(folderpath->Begin(), folderpath->End());

			StorageFile ^romFile = ROMFile;
			wstring wRomName(ROMFile->Name->Begin(), ROMFile->Name->Length() - 4);
			string romName(wRomName.begin(), wRomName.end());

			stringstream tmpFileNameStream;
			tmpFileNameStream << folderPathStr << "\\";
			tmpFileNameStream << romName << SavestateSlot << ".sgm";
			string fileNameA = tmpFileNameStream.str();

			FILE *file;
			auto error = fopen_s(&file, fileNameA.c_str(), "wb");
			if(!file)
			{
#if _DEBUG
				stringstream ss;
				ss << "Unable to open file '";
				ss << fileNameA;
				ss << "' to store savestate (";
				ss << error;
				ss << ").";
				OutputDebugStringA(ss.str().c_str());
				return;
#endif
			}
			fclose(file);

			ofstream stream (fileNameA.c_str(), ios::binary);
			if(!stream.is_open())
			{
#if _DEBUG
				stringstream ss;
				ss << "Unable to open file '";
				ss << fileNameA;
				ss << "' to store savestate.";
				OutputDebugStringA(ss.str().c_str());
				return;
#endif
			}

			extern Gb_Apu *gb_apu;
			extern gb_apu_state_ss state;
			extern variable_desc saveGameStruct[116];
			extern variable_desc eepromSaveData[8];
			extern variable_desc flashSaveData3[6];
			extern variable_desc gba_state[32];
			extern RTCCLOCKDATA rtcClockData;
			extern bool stopState;
			extern int IRQTicks;
			extern int dummy_state [16];

			int version = SAVE_GAME_VERSION;
			stream.write(reinterpret_cast<const char *>(&version), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&rom[0xa0]), 16);
			stream.write(reinterpret_cast<const char *>(&useBios), sizeof(bool));
			stream.write(reinterpret_cast<const char *>(&reg[0]), sizeof(reg));
			int i = 0;
			for (; i < ARRAYSIZE(saveGameStruct); i++)
			{
				if(saveGameStruct[i].size > 0)
				{
					stream.write(reinterpret_cast<const char *>(saveGameStruct[i].address), saveGameStruct[i].size);
				}
			}
			stream.write(reinterpret_cast<const char *>(&stopState), sizeof(bool));
			stream.write(reinterpret_cast<const char *>(&IRQTicks), sizeof(int));
			stream.write(reinterpret_cast<const char *>(internalRAM), 0x8000);
			stream.write(reinterpret_cast<const char *>(paletteRAM), 0x400);
			stream.write(reinterpret_cast<const char *>(workRAM), 0x40000);
			stream.write(reinterpret_cast<const char *>(vram), 0x20000);
			stream.write(reinterpret_cast<const char *>(oam), 0x400);
			//stream.write(reinterpret_cast<const char *>(pix), 0x400);
			stream.write(reinterpret_cast<const char *>(ioMem), 0x400);

			// EEPROM
			for (i = 0; i < ARRAYSIZE(eepromSaveData); i++)
			{
				if(eepromSaveData[i].size > 0)
				{
					stream.write(reinterpret_cast<const char *>(eepromSaveData[i].address), eepromSaveData[i].size);
				}
			}
			stream.write(reinterpret_cast<const char *>(&eepromSize), sizeof(int));
			stream.write(reinterpret_cast<const char *>(eepromData), 0x2000);

			// Flash
			for (i = 0; i < ARRAYSIZE(flashSaveData3); i++)
			{
				if(flashSaveData3[i].size > 0)
				{
					stream.write(reinterpret_cast<const char *>(flashSaveData3[i].address), flashSaveData3[i].size);
				}
			}

			// Sound
			gb_apu->save_state(&state.apu);
			memset(dummy_state, 0, sizeof dummy_state);
			for (i = 0; i < ARRAYSIZE(gba_state); i++)
			{
				if(gba_state[i].size > 0)
				{
					stream.write(reinterpret_cast<const char *>(gba_state[i].address), gba_state[i].size);
				}
			}

			stream.write(reinterpret_cast<const char *>(&rtcClockData), sizeof(rtcClockData));

			stream.flush();
			stream.close();
		}).then([emulator](task<void> t)
		{
			try
			{
				//emulator->Unpause();	
				t.get();
			}catch(Exception ^ex)
			{
#if _DEBUG
				wstring str(ex->Message->Begin(), ex->Message->End());
				OutputDebugStringW((L"Save state: " + str).c_str());
#endif
			}
		});
	}

	//task<void> LoadStateAsync(void)
	//{
	//	if(gbaROMLoaded)
	//	{
	//		return LoadGBAStateAsync();
	//	}else
	//	{
	//		return LoadGBStateAsync();
	//	}
	//}

	task<void> LoadStateAsync(int slot)
	{
		if(gbaROMLoaded)
		{
			return LoadGBAStateAsync(slot);
		}else
		{
			return LoadGBStateAsync(slot);
		}
	}

	task<void> LoadGBStateAsync(int slot)
	{
		int whichslot;
		if (slot <0 ) 
			whichslot = LoadstateSlot;
		else
			whichslot = slot;

		EmulatorGame *emulator = EmulatorGame::GetInstance();
		return create_task([]()
		{
			if(!ROMFile || !ROMFolder)
			{
				throw ref new Exception(E_FAIL, "No ROM loaded.");
			}
			return ROMFolder->GetFolderAsync(SAVE_FOLDER);
		}).then([emulator, whichslot](StorageFolder ^folder)
		{
			emulator->Pause();
			wstringstream extension;

			Platform::String ^tmp = ROMFile->Name;
			const wchar_t *end = tmp->End();
			while(*end != '.') end--;
			size_t diff = tmp->End() - end;

			extension << whichslot << L".sgm";
			Platform::String ^nameWithoutExtension = ref new Platform::String(ROMFile->Name->Data(), ROMFile->Name->Length() - diff);			
			Platform::String ^statePath = folder->Path + "\\" + nameWithoutExtension + ref new Platform::String(extension.str().c_str());
			wstring strPath (statePath->Begin(), statePath->End());

			ifstream stream(strPath, ios::binary);
			if(!stream.is_open())
			{
#if _DEBUG
				wstringstream ss;
				ss << L"Unable to open file '";
				ss << strPath;
				ss << L"' to load savestate.";
				OutputDebugStringW(ss.str().c_str());
#endif
				return;
			}

			extern u8* gbRom;
			extern bool useBios;
			extern bool inBios;
			extern variable_desc gbSaveGameStruct[78];
			extern u16 IFF;
			extern int gbSgbMode;
			extern variable_desc gbSgbSaveStructV3[11];
			extern u8 *gbSgbBorder;
			extern u8 *gbSgbBorderChar;
			extern u8 gbSgbPacket[112];
			extern u16 gbSgbSCPPalette[2048];
			extern u8 gbSgbATF[360];
			extern u8 gbSgbATFList[16200];
			extern mapperMBC1 gbDataMBC1;
			extern mapperMBC2 gbDataMBC2;
			extern mapperMBC3 gbDataMBC3;
			extern mapperMBC5 gbDataMBC5;
			extern mapperHuC1 gbDataHuC1;
			extern mapperHuC3 gbDataHuC3;
			extern mapperTAMA5 gbDataTAMA5;
			extern u8 *gbTAMA5ram;
			extern int gbTAMA5ramSize;
			extern mapperMMM01 gbDataMMM01;
			extern u16 gbPalette[128];
			extern u8 *gbMemory;
			extern int gbRamSize;
			extern u8 *gbRam;
			extern int gbCgbMode;
			extern u8 *gbVram;
			extern u8 *gbWram;
			extern variable_desc gb_state[20];
			extern int gbCheatNumber;
			extern gbCheat gbCheatList[100];
			extern int gbLcdModeDelayed;
			extern int gbLcdTicksDelayed;
			extern int gbLcdLYIncrementTicksDelayed;
			extern u8 gbSpritesTicks[300];
			extern bool gbTimerModeChange;
			extern bool gbTimerOnChange;
			extern int gbHardware;
			extern bool gbBlackScreen;
			extern u8 oldRegister_WY;
			extern int gbWindowLine;
			extern int inUseRegister_WY;
			extern bool gbScreenOn;
			extern int gbSgbMask;
			extern u8 gbSCYLine[300], register_SCY, gbSCXLine[300], register_SCX;
			extern u8 gbBgpLine[300];
			extern u8 gbBgp[4];
			extern u8 gbObp0Line[300], gbObp0[4], gbObp1Line[300], gbObp1[4];
			extern u8 register_SVBK, register_VBK;
			extern int gbBorderOn;
			extern int gbSpeed, gbLine99Ticks;

			int marker = 0x12345678;

			int version;
			stream.read(reinterpret_cast<char *>(&version), sizeof(int));

			u8 romname[20];
			stream.read(reinterpret_cast<char *>(romname), 15);

			int ub, ib;
			stream.read(reinterpret_cast<char *>(&ub), sizeof(int));
			stream.read(reinterpret_cast<char *>(&ib), sizeof(int));
			gbReset();
			inBios = ib ? true : false;

			int i = 0;
			for (; i < ARRAYSIZE(gbSaveGameStruct); i++)
			{
				if(gbSaveGameStruct[i].size > 0)
				{
					stream.read(reinterpret_cast<char *>(gbSaveGameStruct[i].address), gbSaveGameStruct[i].size);
				}
			}

			// Correct crash when loading color gameboy save in regular gameboy type.
			if (!gbCgbMode)
			{
				if(gbVram != NULL) {
					free(gbVram);
					gbVram = NULL;
				}
				if(gbWram != NULL) {
					free(gbWram);
					gbWram = NULL;
				}
			}
			else
			{
				if(gbVram == NULL)
					gbVram = (u8 *)malloc(0x4000);
				if(gbWram == NULL)
					gbWram = (u8 *)malloc(0x8000);
				memset(gbVram,0,0x4000);
				memset(gbPalette,0, 2*128);
			}

			stream.read(reinterpret_cast<char *>(&IFF), sizeof(u16));

			if(gbSgbMode)
			{
				i = 0;
				for (; i < ARRAYSIZE(gbSgbSaveStructV3); i++)
				{
					if(gbSgbSaveStructV3[i].size > 0)
					{
						stream.read(reinterpret_cast<char *>(gbSgbSaveStructV3[i].address), gbSgbSaveStructV3[i].size);
					}
				}
				stream.read(reinterpret_cast<char *>(gbSgbBorder), 2048);
				stream.read(reinterpret_cast<char *>(gbSgbBorderChar), 32*256);

				stream.read(reinterpret_cast<char *>(gbSgbPacket), 16*7);
				stream.read(reinterpret_cast<char *>(gbSgbSCPPalette), 4 * 512 * sizeof(u16));
				stream.read(reinterpret_cast<char *>(gbSgbATF), 20 * 18);
				stream.read(reinterpret_cast<char *>(gbSgbATFList), 45 * 20 * 18);
			}else
			{
				gbSgbMask = 0;
			}

			stream.read(reinterpret_cast<char *>(&gbDataMBC1), sizeof(gbDataMBC1));
			stream.read(reinterpret_cast<char *>(&gbDataMBC2), sizeof(gbDataMBC2));
			stream.read(reinterpret_cast<char *>(&gbDataMBC3), sizeof(gbDataMBC3));
			stream.read(reinterpret_cast<char *>(&gbDataMBC5), sizeof(gbDataMBC5));
			stream.read(reinterpret_cast<char *>(&gbDataHuC1), sizeof(gbDataHuC1));
			stream.read(reinterpret_cast<char *>(&gbDataHuC3), sizeof(gbDataHuC3));
			stream.read(reinterpret_cast<char *>(&gbDataTAMA5), sizeof(gbDataTAMA5));
			if(gbTAMA5ram != NULL)
			{
				if(skipSaveGameBattery)
				{
					stream.seekg(gbTAMA5ramSize, ios_base::cur);

				}else
				{
					stream.read(reinterpret_cast<char *>(gbTAMA5ram), gbTAMA5ramSize);
				}
			}
			stream.read(reinterpret_cast<char *>(&gbDataMMM01), sizeof(gbDataMMM01));

			stream.read(reinterpret_cast<char *>(gbPalette), 128 * sizeof(u16));

			stream.read(reinterpret_cast<char *>(&gbMemory[0x8000]), 0x8000);

			if(gbRamSize && gbRam)
			{
				int ramSize;
				stream.read(reinterpret_cast<char *>(&ramSize), sizeof(int));
				if(skipSaveGameBattery)
				{
					stream.seekg((gbRamSize>ramSize) ? ramSize : gbRamSize, ios_base::cur);
				}else
				{
					stream.read(reinterpret_cast<char *>(gbRam), (gbRamSize>ramSize) ? ramSize : gbRamSize);
				}
				if(ramSize > gbRamSize)
				{
					stream.seekg(ramSize-gbRamSize, ios_base::cur);
				}
			}

			memset(gbSCYLine, register_SCY, sizeof(gbSCYLine));
			memset(gbSCXLine, register_SCX, sizeof(gbSCXLine));
			memset(gbBgpLine, (gbBgp[0] | (gbBgp[1]<<2) | (gbBgp[2]<<4) |
				(gbBgp[3]<<6)), sizeof(gbBgpLine));
			memset(gbObp0Line, (gbObp0[0] | (gbObp0[1]<<2) | (gbObp0[2]<<4) |
				(gbObp0[3]<<6)), sizeof(gbObp0Line));
			memset(gbObp1Line, (gbObp1[0] | (gbObp1[1]<<2) | (gbObp1[2]<<4) |
				(gbObp1[3]<<6)), sizeof(gbObp1Line));
			memset(gbSpritesTicks, 0x0, sizeof(gbSpritesTicks));

			if (inBios)
			{
				gbMemoryMap[0x00] = &gbMemory[0x0000];
				memcpy ((u8 *)(gbMemory), (u8 *)(gbRom), 0x1000);
				memcpy ((u8 *)(gbMemory), (u8 *)(bios), 0x100);
			}
			else 
			{ 
				gbMemoryMap[0x00] = &gbRom[0x0000];			
			}
			gbMemoryMap[0x01] = &gbRom[0x1000];
			gbMemoryMap[0x02] = &gbRom[0x2000];
			gbMemoryMap[0x03] = &gbRom[0x3000];
			gbMemoryMap[0x04] = &gbRom[0x4000];
			gbMemoryMap[0x05] = &gbRom[0x5000];
			gbMemoryMap[0x06] = &gbRom[0x6000];
			gbMemoryMap[0x07] = &gbRom[0x7000];
			gbMemoryMap[0x08] = &gbMemory[0x8000];
			gbMemoryMap[0x09] = &gbMemory[0x9000];
			gbMemoryMap[0x0a] = &gbMemory[0xa000];
			gbMemoryMap[0x0b] = &gbMemory[0xb000];
			gbMemoryMap[0x0c] = &gbMemory[0xc000];
			gbMemoryMap[0x0d] = &gbMemory[0xd000];
			gbMemoryMap[0x0e] = &gbMemory[0xe000];
			gbMemoryMap[0x0f] = &gbMemory[0xf000];

			switch(gbRomType) 
			{
			case 0x00:
			case 0x01:
			case 0x02:
			case 0x03:
				// MBC 1
				memoryUpdateMapMBC1();
				break;
			case 0x05:
			case 0x06:
				// MBC2
				memoryUpdateMapMBC2();
				break;
			case 0x0b:
			case 0x0c:
			case 0x0d:
				// MMM01
				memoryUpdateMapMMM01();
				break;
			case 0x0f:
			case 0x10:
			case 0x11:
			case 0x12:
			case 0x13:
				// MBC 3
				memoryUpdateMapMBC3();
				break;
			case 0x19:
			case 0x1a:
			case 0x1b:
				// MBC5
				memoryUpdateMapMBC5();
				break;
			case 0x1c:
			case 0x1d:
			case 0x1e:
				// MBC 5 Rumble
				memoryUpdateMapMBC5();
				break;
			case 0x22:
				// MBC 7
				memoryUpdateMapMBC7();
				break;
			case 0x56:
				// GS3
				memoryUpdateMapGS3();
				break;
			case 0xfd:
				// TAMA5
				memoryUpdateMapTAMA5();
				break;
			case 0xfe:
				// HuC3
				memoryUpdateMapHuC3();
				break;
			case 0xff:
				// HuC1
				memoryUpdateMapHuC1();
				break;
			}

			if(gbCgbMode)
			{
				stream.read(reinterpret_cast<char *>(gbVram), 0x4000);
				stream.read(reinterpret_cast<char *>(gbWram), 0x8000);

				int value = register_SVBK;
				if(value == 0)
					value = 1;

				gbMemoryMap[0x08] = &gbVram[register_VBK * 0x2000];
				gbMemoryMap[0x09] = &gbVram[register_VBK * 0x2000 + 0x1000];
				gbMemoryMap[0x0d] = &gbWram[value * 0x1000];
			}

			gbSoundReadGame2();
			i = 0;
			for (; i < ARRAYSIZE(gb_state); i++)
			{
				if(gb_state[i].size > 0)
				{
					stream.read(reinterpret_cast<char *>(gb_state[i].address), gb_state[i].size);
				}
			}
			gbSoundReadGame3();

			if (gbCgbMode && gbSgbMode) {
				gbSgbMode = 0;
			}

			if(gbBorderOn && !gbSgbMask) {
				gbSgbRenderBorder();
			}

			// systemDrawScreen(); // Deadlock!

			int numberCheats;
			stream.read(reinterpret_cast<char *>(&numberCheats), sizeof(int));
			if(skipSaveGameCheats)
			{
				stream.seekg(numberCheats * sizeof(gbCheat), ios_base::cur);
			}else
			{
				gbCheatNumber = numberCheats;
				if(gbCheatNumber > 0)
				{
					stream.read(reinterpret_cast<char *>(&gbCheatList[0]), sizeof(gbCheat) * gbCheatNumber);
				}
			}
			gbCheatUpdateMap();

			int spriteTicks;
			int timerModeChange;
			int timerOnChange;
			int blackScreen;
			int oldRegister;
			int screenOn;
			stream.read(reinterpret_cast<char *>(&gbLcdModeDelayed), sizeof(int));
			stream.read(reinterpret_cast<char *>(&gbLcdTicksDelayed), sizeof(int));
			stream.read(reinterpret_cast<char *>(&gbLcdLYIncrementTicksDelayed), sizeof(int));
			stream.read(reinterpret_cast<char *>(&spriteTicks), sizeof(int));
			stream.read(reinterpret_cast<char *>(&timerModeChange), sizeof(int));
			stream.read(reinterpret_cast<char *>(&timerOnChange), sizeof(int));
			stream.read(reinterpret_cast<char *>(&gbHardware), sizeof(int));
			stream.read(reinterpret_cast<char *>(&blackScreen), sizeof(int));
			stream.read(reinterpret_cast<char *>(&oldRegister), sizeof(int));
			stream.read(reinterpret_cast<char *>(&gbWindowLine), sizeof(int));
			stream.read(reinterpret_cast<char *>(&inUseRegister_WY), sizeof(int));
			stream.read(reinterpret_cast<char *>(&screenOn), sizeof(int));
			gbSpritesTicks[299] = spriteTicks;
			gbTimerModeChange = timerModeChange ? true : false;
			gbTimerOnChange = timerOnChange ? true : false;
			gbBlackScreen = blackScreen ? true : false;
			oldRegister_WY = oldRegister;
			gbScreenOn = screenOn;


			if (gbSpeed)
				gbLine99Ticks *= 2;

			systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;
			
			if(cheatsTmp != nullptr)
			{
				LoadCheats(cheatsTmp);
				cheatsTmp = nullptr;
			}

		}).then([](){}).then([emulator](task<void> t)
		{
			try
			{
				t.get();
			}catch(Platform::Exception ^ex)
			{
#if _DEBUG
				wstring err = ex->Message->Data();
				OutputDebugStringW((L"Load state: " + err).c_str());
#endif
			}
		});
	}

	task<void> LoadGBAStateAsync(int slot)
	{
		int whichslot;
		if (slot <0 ) 
			whichslot = LoadstateSlot;
		else
			whichslot = slot;

		EmulatorGame *emulator = EmulatorGame::GetInstance();
		return create_task([]()
		{
			if(!ROMFile || !ROMFolder)
			{
				throw ref new Exception(E_FAIL, "No ROM loaded.");
			}
			return ROMFolder->GetFolderAsync(SAVE_FOLDER);
		}).then([emulator, whichslot](StorageFolder ^folder)
		{
			emulator->Pause();
			wstringstream extension;

			extension << whichslot << L".sgm";
			Platform::String ^nameWithoutExtension = ref new Platform::String(ROMFile->Name->Data(), ROMFile->Name->Length() - 4);			
			Platform::String ^statePath = folder->Path + "\\" + nameWithoutExtension + ref new Platform::String(extension.str().c_str());
			wstring strPath (statePath->Begin(), statePath->End());

			ifstream stream(strPath, ios::binary);
			if(!stream.is_open())
			{
#if _DEBUG
				wstringstream ss;
				ss << L"Unable to open file '";
				ss << strPath;
				ss << L"' to load savestate.";
				OutputDebugStringW(ss.str().c_str());
#endif
				return;
			}

			extern Gb_Apu *gb_apu;
			extern gb_apu_state_ss state;
			extern variable_desc saveGameStruct[116];
			extern variable_desc eepromSaveData[8];
			extern variable_desc flashSaveData3[6];
			extern variable_desc gba_state[32];
			extern RTCCLOCKDATA rtcClockData;
			extern bool stopState;
			extern int IRQTicks;
			extern int dummy_state [16];
			extern bool intState;

			int version;
			u8 romname[17];
			romname[16] = 0;
			bool ub;
			stream.read(reinterpret_cast<char *>(&version), sizeof(int));
			stream.read(reinterpret_cast<char *>(romname), 16);
			stream.read(reinterpret_cast<char *>(&ub), sizeof(bool));
			stream.read(reinterpret_cast<char *>(&reg[0]), sizeof(reg));
			int i = 0;
			for (; i < ARRAYSIZE(saveGameStruct); i++)
			{
				if(saveGameStruct[i].size > 0)
				{
					stream.read(reinterpret_cast<char *>(saveGameStruct[i].address), saveGameStruct[i].size);
				}
			}
			stream.read(reinterpret_cast<char *>(&stopState), sizeof(bool));
			stream.read(reinterpret_cast<char *>(&IRQTicks), sizeof(int));
			if(IRQTicks > 0)
			{
				intState = true;
			}else
			{
				intState = false;
				IRQTicks = 0;
			}
			stream.read(reinterpret_cast<char *>(internalRAM), 0x8000);
			stream.read(reinterpret_cast<char *>(paletteRAM), 0x400);
			stream.read(reinterpret_cast<char *>(workRAM), 0x40000);
			stream.read(reinterpret_cast<char *>(vram), 0x20000);
			stream.read(reinterpret_cast<char *>(oam), 0x400);
			stream.read(reinterpret_cast<char *>(ioMem), 0x400);


			if(skipSaveGameBattery)
			{
				// Skip EEPROM
				for (i = 0; i < ARRAYSIZE(eepromSaveData); i++)
				{
					stream.seekg(eepromSaveData[i].size, ios_base::cur);
				}
				stream.seekg(sizeof(int), ios_base::cur);
				stream.seekg(0x2000, ios_base::cur);

				// Skip Flash
				for (i = 0; i < ARRAYSIZE(flashSaveData3); i++)
				{
					stream.seekg(flashSaveData3[i].size, ios_base::cur);
				}

			}else
			{
				// Read EEPROM
				for (i = 0; i < ARRAYSIZE(eepromSaveData); i++)
				{
					if(eepromSaveData[i].size > 0)
					{
						stream.read(reinterpret_cast<char *>(eepromSaveData[i].address), eepromSaveData[i].size);
					}
				}
				stream.read(reinterpret_cast<char *>(&eepromSize), sizeof(int));
				stream.read(reinterpret_cast<char *>(eepromData), 0x2000);

				// Read Flash
				for (i = 0; i < ARRAYSIZE(flashSaveData3); i++)
				{
					if(flashSaveData3[i].size > 0)
					{
						stream.read(reinterpret_cast<char *>(flashSaveData3[i].address), flashSaveData3[i].size);
					}
				}
			}

			// Sound
			reset_apu();
			gb_apu->save_state(&state.apu);
			for (i = 0; i < ARRAYSIZE(gba_state); i++)
			{
				if(gba_state[i].size > 0)
				{
					stream.read(reinterpret_cast<char *>(gba_state[i].address), gba_state[i].size);
				}
			}
			gb_apu->load_state(state.apu);
			write_SGCNT0_H(READ16LE(&ioMem[SGCNT0_H]) & 0x770F);
			apply_muting();

			stream.read(reinterpret_cast<char *>(&rtcClockData), sizeof(rtcClockData));

			stream.close();

			layerEnable = layerSettings & DISPCNT;
			CPUUpdateRender();
			CPUUpdateRenderBuffers(true);
			CPUUpdateWindow0();
			CPUUpdateWindow1();

			gbaSaveType = 0;
			switch(saveType) {
			case 0:
				cpuSaveGameFunc = flashSaveDecide;
				break;
			case 1:
				cpuSaveGameFunc = sramWrite;
				gbaSaveType = 1;
				break;
			case 2:
				cpuSaveGameFunc = flashWrite;
				gbaSaveType = 2;
				break;
			case 3:
				break;
			case 5:
				gbaSaveType = 5;
				break;
			default:
				systemMessage(MSG_UNSUPPORTED_SAVE_TYPE,
					N_("Unsupported save type %d"), saveType);
				break;
			}
			if(eepromInUse)
				gbaSaveType = 3;

			CPUReadHelper();

			if(cheatsTmp != nullptr)
			{
				LoadCheats(cheatsTmp);
				cheatsTmp = nullptr;
			}

		}).then([](){}).then([emulator](task<void> t)
		{
			try
			{
				t.get();
			}catch(Platform::Exception ^ex)
			{
#if _DEBUG
				wstring err = ex->Message->Data();
				OutputDebugStringW((L"Load state: " + err).c_str());
#endif
			}
		});
	}

	task<void> SaveSRAMAsync ()
	{
		if(gbaROMLoaded)
		{
			return SaveGBASRAMAsync();
		}else
		{
			return SaveGBSRAMAsync();
		}
	}

	task<void> SaveGBSRAMAsync(void)
	{
		if(ROMFile == nullptr || ROMFolder == nullptr)
			return task<void>();

		Platform::String ^name = ROMFile->Name;
		const wchar_t *end = name->End();
		while(*end != '.') end--;
		size_t diff = name->End() - end;

		Platform::String ^nameWithoutExt = ref new Platform::String(name->Begin(), name->Length() - diff);
		Platform::String ^sramName = nameWithoutExt->Concat(nameWithoutExt, ".sav");


		return create_task([]()
		{
			return ROMFolder->CreateFolderAsync(SAVE_FOLDER, CreationCollisionOption::OpenIfExists);
		}).then([sramName](StorageFolder ^saveFolder)
		{
			// try to open the file			
			return saveFolder->CreateFileAsync(sramName, CreationCollisionOption::OpenIfExists);
		}).then([](StorageFile ^file)
		{
			if(gbBattery)
			{
				switch(gbRomType)
				{
				case 0xff:
				case 0x03:
					// MBC1
					if(gbRam)
					{
						SaveBytesToFileAsync(file, gbRam, gbRamSizeMask + 1).wait();
					}
					break;
				case 0x06:
					// MBC2
					if(gbRam)
					{
						SaveBytesToFileAsync(file, gbMemoryMap[0x0a], 512).wait();
					}
					break;
				case 0x0d:
					// MMM01
					if(gbRam)
					{
						SaveBytesToFileAsync(file, gbRam, gbRamSizeMask + 1).wait();
					}
					break;
				case 0x0f:
				case 0x10:
					// MBC3
					if(gbRam)
					{
						int tmpSize = gbRamSizeMask + 1 + 10 * sizeof(int) + sizeof(time_t);
						u8 *tmp = new u8[tmpSize];
						memcpy_s(tmp, gbRamSizeMask + 1, gbRam, gbRamSizeMask + 1);
						memcpy_s(tmp + gbRamSizeMask + 1, 10 * sizeof(int) + sizeof(time_t), &gbDataMBC3.mapperSeconds, 10 * sizeof(int) + sizeof(time_t));

						SaveBytesToFileAsync(file, tmp, tmpSize).wait();

						delete [] tmp;
					}else
					{
						SaveBytesToFileAsync(file, (u8 *) &gbDataMBC3.mapperSeconds, 10 * sizeof(int) + sizeof(time_t)).wait();
					}
					break;
				case 0x13:
				case 0xfc:
					// MBC3 - 2
					if(gbRam)
					{
						SaveBytesToFileAsync(file, gbRam, gbRamSizeMask + 1).wait();
					}
					break;
				case 0x1b:
				case 0x1e:
					// MBC5
					if(gbRam)
					{
						SaveBytesToFileAsync(file, gbRam, gbRamSizeMask + 1).wait();
					}
					break;
				case 0x22:
					// MBC7
					if(gbRam)
					{
						SaveBytesToFileAsync(file, &gbMemory[0xa000], 256).wait();
					}
					break;
				case 0xfd:
					if(gbRam)
					{
						int tmpSize = gbRamSizeMask + 1 + gbTAMA5ramSize + 14 * sizeof(int) + sizeof(time_t);
						u8 *tmp = new u8[tmpSize];
						memcpy_s(tmp, gbRamSizeMask + 1, gbRam, gbRamSizeMask + 1);
						memcpy_s(tmp + gbRamSizeMask + 1, gbTAMA5ramSize, gbTAMA5ram, gbTAMA5ramSize);
						memcpy_s(tmp + gbRamSizeMask + 1 + gbTAMA5ramSize, 14 * sizeof(int) + sizeof(time_t), &gbDataTAMA5.mapperSeconds, 14 * sizeof(int) + sizeof(time_t));

						SaveBytesToFileAsync(file, tmp, tmpSize).wait();

						delete [] tmp;
					}else
					{
						int tmpSize = gbTAMA5ramSize + 14 * sizeof(int) + sizeof(time_t);
						u8 *tmp = new u8[tmpSize];
						memcpy_s(tmp, gbTAMA5ramSize, gbTAMA5ram, gbTAMA5ramSize);
						memcpy_s(tmp + gbTAMA5ramSize, 14 * sizeof(int) + sizeof(time_t), &gbDataTAMA5.mapperSeconds, 14 * sizeof(int) + sizeof(time_t));

						SaveBytesToFileAsync(file, tmp, tmpSize).wait();

						delete [] tmp;
					}
					break;
				}
			}
		}).then([sramName](task<void> t)
		{
			try
			{
				t.get();
			}catch(Platform::COMException ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(sramName->Begin(), sramName->End());
				wstring werror(error->Begin(), error->End());
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}
		});
	}

	task<void> SaveGBASRAMAsync(void)
	{
		if(ROMFile == nullptr || ROMFolder == nullptr)
			return task<void>();

		Platform::String ^name = ROMFile->Name;
		Platform::String ^nameWithoutExt = ref new Platform::String(name->Begin(), name->Length() - 4);
		Platform::String ^sramName = nameWithoutExt->Concat(nameWithoutExt, ".sav");


		return create_task([]()
		{
			return ROMFolder->CreateFolderAsync(SAVE_FOLDER, CreationCollisionOption::OpenIfExists);
		}).then([sramName](StorageFolder ^saveFolder)
		{
			// try to open the file			
			return saveFolder->CreateFileAsync(sramName, CreationCollisionOption::OpenIfExists);
		}).then([](StorageFile ^file)
		{
			if(gbaSaveType == 0) {
				if(eepromInUse)
					gbaSaveType = 3;
				else switch(saveType) {
				case 1:
					gbaSaveType = 1;
					break;
				case 2:
					gbaSaveType = 2;
					break;
				}
			}

			if((gbaSaveType) && (gbaSaveType!=5)) 
			{
				// only save if Flash/Sram in use or EEprom in use
				if(gbaSaveType != 3) 
				{
					if(gbaSaveType == 2) 
					{
						SaveBytesToFileAsync(file, flashSaveMemory, flashSize).wait();
					} 
					else 
					{
						SaveBytesToFileAsync(file, flashSaveMemory, 0x10000).wait();
					}
				} 
				else 
				{
					SaveBytesToFileAsync(file, eepromData, eepromSize).wait();
				}
			}

		}).then([sramName](task<void> t)
		{
			try
			{
				t.get();
			}catch(Platform::COMException ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(sramName->Begin(), sramName->End());
				wstring werror(error->Begin(), error->End());
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}
		});
	}

	task<void> LoadSRAMAsync ()
	{
		if(gbaROMLoaded)
		{
			return LoadGBASRAMAsync();
		}else
		{
			return LoadGBSRAMAsync();
		}
	}

	task<void> LoadGBSRAMAsync ()
	{
		if(!ROMFile || !ROMFolder)
			return task<void>([](){});

		Platform::String ^name = ROMFile->Name;
		const wchar_t *end = name->End();
		while(*end != '.') end--;
		size_t diff = name->End() - end;

		Platform::String ^nameWithoutExt = ref new Platform::String(name->Begin(), name->Length() - diff);
		Platform::String ^sramName = nameWithoutExt->Concat(nameWithoutExt, ".sav");

		return create_task([]()
		{
			return ROMFolder->GetFolderAsync(SAVE_FOLDER);
		}).then([sramName](StorageFolder ^folder)
		{
			// try to open the file
			return folder->GetFileAsync(sramName);
		})
			.then([](StorageFile ^file)
		{
			// get bytes out of storage file
			return GetBytesFromFileAsync(file);
		}).then([](ROMData data)
		{
			if(gbBattery)
			{
				switch(gbRomType)
				{
				case 0x03:
					// MBC1
					if(gbRam)
					{
						memcpy_s(gbRam, gbRamSizeMask + 1, data.ROM, gbRamSizeMask + 1);
					}
					break;
				case 0x06:
					// MBC2
					if(gbRam)
					{
						memcpy_s(gbMemoryMap[0x0a], 512, data.ROM, 512);
					}
					break;
				case 0x0d:
					// MMM01
					if(gbRam)
					{
						memcpy_s(gbRam, gbRamSizeMask + 1, data.ROM, gbRamSizeMask + 1);
					}
					break;
				case 0x0f:
				case 0x10:
					// MBC3
					try{
						if(gbRam)
						{
							memcpy_s(gbRam, gbRamSizeMask + 1, data.ROM, gbRamSizeMask + 1);
							memcpy_s(&gbDataMBC3.mapperSeconds, sizeof(int) * 10 + sizeof(time_t), data.ROM + gbRamSizeMask + 1, sizeof(int) * 10 + sizeof(time_t));
						}else
						{
							memcpy_s(&gbDataMBC3.mapperSeconds, sizeof(int) * 10 + sizeof(time_t), data.ROM, sizeof(int) * 10 + sizeof(time_t));
						}
					}catch(...)
					{
						time(&gbDataMBC3.mapperLastTime);
						struct tm *lt;
						lt = localtime(&gbDataMBC3.mapperLastTime);
						gbDataMBC3.mapperSeconds = lt->tm_sec;
						gbDataMBC3.mapperMinutes = lt->tm_min;
						gbDataMBC3.mapperHours = lt->tm_hour;
						gbDataMBC3.mapperDays = lt->tm_yday & 255;
						gbDataMBC3.mapperControl = (gbDataMBC3.mapperControl & 0xfe) |
							(lt->tm_yday > 255 ? 1: 0);
					}
					break;
				case 0x13:
				case 0xfc:
					// MBC3 - 2
					if(gbRam)
					{
						memcpy_s(gbRam, gbRamSizeMask + 1, data.ROM, gbRamSizeMask + 1);
						memcpy_s(&gbDataMBC3.mapperSeconds, sizeof(int) * 10 + sizeof(time_t), data.ROM + gbRamSizeMask + 1, sizeof(int) * 10 + sizeof(time_t));
					}else
					{
						memcpy_s(&gbDataMBC3.mapperSeconds, sizeof(int) * 10 + sizeof(time_t), data.ROM, sizeof(int) * 10 + sizeof(time_t));
					}
					break;
				case 0x1b:
				case 0x1e:
					// MBC5
					if(gbRam)
					{
						memcpy_s(gbRam, gbRamSizeMask + 1, data.ROM, gbRamSizeMask + 1);
					}
					break;
				case 0x22:
					// MBC7
					if(gbRam)
					{
						memcpy_s(&gbMemory[0xa000], 256, data.ROM, 256);
					}
					break;
				case 0xfd:
					try
					{
						if(gbRam)
						{
							memcpy_s(gbRam, gbRamSizeMask + 1, data.ROM, gbRamSizeMask + 1);
							memcpy_s(gbTAMA5ram, gbTAMA5ramSize, data.ROM + gbRamSizeMask + 1, gbTAMA5ramSize);
							memcpy_s(&gbDataTAMA5.mapperSeconds, sizeof(int)*14 + sizeof(time_t), data.ROM + gbRamSizeMask + 1 + gbTAMA5ramSize, sizeof(int)*14 + sizeof(time_t));
						}else
						{
							memcpy_s(gbTAMA5ram, gbTAMA5ramSize, data.ROM, gbTAMA5ramSize);
							memcpy_s(&gbDataTAMA5.mapperSeconds, sizeof(int)*14 + sizeof(time_t), data.ROM + gbTAMA5ramSize, sizeof(int)*14 + sizeof(time_t));
						}
					}catch(...)
					{
						u8 gbDaysinMonth [12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
						time(&gbDataTAMA5.mapperLastTime);
						struct tm *lt;
						lt = localtime(&gbDataTAMA5.mapperLastTime);
						gbDataTAMA5.mapperSeconds = lt->tm_sec;
						gbDataTAMA5.mapperMinutes = lt->tm_min;
						gbDataTAMA5.mapperHours = lt->tm_hour;
						gbDataTAMA5.mapperDays = 1;
						gbDataTAMA5.mapperMonths = 1;
						gbDataTAMA5.mapperYears = 1970;
						int days = lt->tm_yday+365*3;
						while (days)
						{
							gbDataTAMA5.mapperDays++;
							days--;
							if (gbDataTAMA5.mapperDays>gbDaysinMonth[gbDataTAMA5.mapperMonths-1])
							{
								gbDataTAMA5.mapperDays = 1;
								gbDataTAMA5.mapperMonths++;
								if (gbDataTAMA5.mapperMonths>12)
								{
									gbDataTAMA5.mapperMonths = 1;
									gbDataTAMA5.mapperYears++;
									if ((gbDataTAMA5.mapperYears & 3) == 0)
										gbDaysinMonth[1] = 29;
									else
										gbDaysinMonth[1] = 28;
								}
							}
						}
						gbDataTAMA5.mapperControl = (gbDataTAMA5.mapperControl & 0xfe) |
							(lt->tm_yday > 255 ? 1: 0);
					}
					break;
				}
			}

			delete [] data.ROM;

		}).then([sramName](task<void> t)
		{
			// handle exceptions
			try
			{
				t.get();
			}
			catch(Platform::COMException ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(sramName->Begin(), sramName->End());
				wstring werror(error->Begin(), error->End());
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}catch(Platform::AccessDeniedException ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(sramName->Begin(), sramName->End());
				wstring werror(error->Begin(), error->End());
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}catch(Platform::Exception ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(sramName->Begin(), sramName->End());
				wstring werror(error->Begin(), error->End());
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}
		});	
	}

	task<void> LoadGBASRAMAsync ()
	{
		if(!ROMFile || !ROMFolder)
			return task<void>([](){});

		Platform::String ^name = ROMFile->Name;
		Platform::String ^nameWithoutExt = ref new Platform::String(name->Begin(), name->Length() - 4);
		Platform::String ^sramName = nameWithoutExt->Concat(nameWithoutExt, ".sav");

		return create_task([]()
		{
			return ROMFolder->GetFolderAsync(SAVE_FOLDER);
		}).then([sramName](StorageFolder ^folder)
		{
			// try to open the file
			return folder->GetFileAsync(sramName);
		})
			.then([](StorageFile ^file)
		{
			// get bytes out of storage file
			return GetBytesFromFileAsync(file);
		}).then([](ROMData data)
		{
			systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

			if(data.Length == 512 || data.Length == 0x2000) 
			{
				memcpy_s(eepromData, data.Length, data.ROM, data.Length);
			} 
			else 
			{
				if(data.Length == 0x20000) 
				{
					memcpy_s(flashSaveMemory, 0x20000, data.ROM, 0x20000);
					flashSetSize(0x20000);
				} 
				else if(data.Length >= 0x10000)
				{
					memcpy_s(flashSaveMemory, 0x10000, data.ROM, 0x10000);
					flashSetSize(0x10000);
				}
			}

			delete [] data.ROM;

		}).then([sramName](task<void> t)
		{
			// handle exceptions
			try
			{
				t.get();
			}
			catch(Platform::COMException ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(sramName->Begin(), sramName->End());
				wstring werror(error->Begin(), error->End());
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}catch(Platform::AccessDeniedException ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(sramName->Begin(), sramName->End());
				wstring werror(error->Begin(), error->End());
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}catch(Platform::Exception ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(sramName->Begin(), sramName->End());
				wstring werror(error->Begin(), error->End());
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}
		});	
	}

	task<void> SaveBytesToFileAsync(StorageFile ^file, unsigned char *bytes, size_t length)
	{
		Platform::String ^name = file->Name;

		return create_task(file->OpenAsync(FileAccessMode::ReadWrite))
			.then([=](IRandomAccessStream ^stream)
		{
			IOutputStream ^outputStream = stream->GetOutputStreamAt(0L);;
			DataWriter ^writer = ref new DataWriter(outputStream);

			Platform::Array<unsigned char> ^array = ref new Array<unsigned char>(length);
			memcpy(array->Data, bytes, length);

			writer->WriteBytes(array);
			create_task(writer->StoreAsync()).wait();
			writer->DetachStream();
			return create_task(outputStream->FlushAsync());
		}).then([name](bool b)
		{ 
			if(!b)
			{
#if _DEBUG
				OutputDebugStringW(L"Error while writing files to file.");
#endif
			}
		}).then([name](task<void> t)
		{
			try
			{
				t.get();
			}catch(COMException ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(name->Begin(), name->End());
				wstring werror(error->Begin(), error->End());
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}
		});
	}

	task<ROMData> GetBytesFromFileAsync(StorageFile ^file)
	{
		auto inputStream = make_shared<IInputStream ^>();
		auto openTask = create_task(file->OpenSequentialReadAsync());

		return openTask.then([=] (IInputStream ^stream)
		{ 
			*inputStream = stream;
			return file->GetBasicPropertiesAsync();
		}).then([=](BasicProperties ^properties)
		{
			Buffer ^buffer = ref new Buffer(properties->Size);
			return (*inputStream)->ReadAsync(buffer, properties->Size, InputStreamOptions::None);
		})
			.then([=](IBuffer ^buffer)
		{			
			DataReader ^reader = DataReader::FromBuffer(buffer);
			Array<BYTE> ^bytes = ref new Array<BYTE>(buffer->Length);
			reader->ReadBytes(bytes);
			BYTE *rawBytes = new BYTE[buffer->Length];
			for (int i = 0; i < buffer->Length; i++)
			{
				rawBytes[i] = bytes[i]; 
			}

			ROMData data;
			data.Length = buffer->Length;
			data.ROM = rawBytes;

			return data;
		});
	}

	void ResetSync(void)
	{
		
		
		if(!ROMFile || !ROMFolder)
			return;

		EmulatorGame *emulator = EmulatorGame::GetInstance();
		emulator->Pause();
		gbexecute = false;

		EmulatorGame::emulator.emuReset();
		emulator->Unpause();
	}

	//task<void> ResetAsync(void)
	//{
	//	if(!ROMFile || !ROMFolder)
	//		return task<void>();

	//	

	//	return LoadROMAsync(ROMFile, ROMFolder);
	//}

	task<void> LoadROMAsync(StorageFile ^file, StorageFolder ^folder)
	{
		

		bool gba = false;
		String^ filename = file->Name;
		if(filename->Length() >= 3)
		{
			const wchar_t *end = filename->End();
			const wchar_t *extStart = end - 3;
			if(extStart >= filename->Begin())
			{
				gba = ((extStart[0] == 'g' || extStart[0] == 'G') && 
					(extStart[1] == 'b' || extStart[1] == 'B') &&
					(extStart[2] == 'a' || extStart[2] == 'A'));
			}
		}
		if(gba)
		{
			return LoadGBAROMAsync(file, folder);
		}else
		{
			return LoadGBROMAsync(file, folder);
		}
	}




	task<void> LoadGBROMAsync(StorageFile ^file, StorageFolder ^folder)
	{
		EmulatorGame *emulator = EmulatorGame::GetInstance();
		
		
		gbexecute = false; //added by Duc Le

		return create_task([emulator]()
		{
			
			emulator->Pause();
			emulator->StopEmulationAsync().wait();
			
		}).then([file]()
		{

			return GetROMBytesFromFileAsync(file);
		}).then([emulator, file, folder](ROMData data)
		{
			int size = data.Length;

			if(rom != NULL) {
				EmulatorGame::emulator.emuCleanUp; // CPUCleanUp();
			}

			extern int turboSkip;
			turboSkip = EmulatorSettings::Current->TurboFrameSkip;


			systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

			extern u8 *gbRom;
			extern int gbRomSize;
			extern bool gbBatteryError;
			extern int gbHardware;
			extern int gbBorderOn;
			extern int gbCgbMode;

			for(int i = 0; i < 24;) 
			{
				systemGbPalette[i++] = (0x1f) | (0x1f << 5) | (0x1f << 10);
				systemGbPalette[i++] = (0x15) | (0x15 << 5) | (0x15 << 10);
				systemGbPalette[i++] = (0x0c) | (0x0c << 5) | (0x0c << 10);
				systemGbPalette[i++] = 0;
			}

			gbRom = (u8 *) malloc(size);
			if(gbRom == NULL) {
				systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
					"ROM");
				CPUCleanUp();
			}  

			memcpy_s(gbRom, size, data.ROM, size);

			if(data.ROM)
			{
				delete [] data.ROM;
			}

			gbRomSize = size;
			gbBatteryError = false;

			if(bios != NULL) {
				free(bios);
				bios = NULL;
			}
			bios = (u8 *)calloc(1,0x100);

			gbUpdateSizes();
			gbGetHardwareType();

			gbReset();			
			EmulatorGame::emulator = GBSystem;
			gbBorderOn = false;

			soundInit();
			gbSoundReset();

			ROMSize = size;
			ROMFile = file;
			ROMFolder = folder;
			gbaROMLoaded = false;


		}).then([]()
		{
			return LoadSRAMAsync();
		}).then([emulator]()
		{			
			if(cheatsTmp != nullptr)
			{
				LoadCheats(cheatsTmp);
				cheatsTmp = nullptr;
			}
			emulator->Unpause();

			emulator->Start();
			VirtualController::GetInstance()->UpdateFormat(VirtualController::GetInstance()->GetFormat());
		})
			.then([](task<void> t)
		{
			try
			{
				t.get();
			}catch(Platform::Exception ^ex)
			{
				if(ex->HResult != E_FAIL)
				{
					throw ex;
				}
			}
		});
	}



	task<void> LoadGBAROMAsync(StorageFile ^file, StorageFolder ^folder)
	{
		EmulatorGame *emulator = EmulatorGame::GetInstance();
		return create_task([emulator]()
		{
			emulator->Pause();
			emulator->StopEmulationAsync().wait();
		}).then([file]()
		{
			CPUCleanUp();
			return GetROMBytesFromFileAsync(file);
		}).then([emulator, file, folder](ROMData data)
		{
			int size = 0x2000000;

			systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

			rom = (u8 *)malloc(0x2000000);
			if(rom == NULL) {
				systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
					"ROM");
			}
			workRAM = (u8 *)calloc(1, 0x40000);
			if(workRAM == NULL) {
				systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
					"WRAM");
			}

			u8 *whereToLoad = rom;
			if(cpuIsMultiBoot)
				whereToLoad = workRAM;

			int read = size = data.Length < size ? data.Length : size;
			memcpy_s(whereToLoad, read, data.ROM, read);

			u16 *temp = (u16 *)(rom+((size+1)&~1));
			int i;
			for(i = (size+1)&~1; i < 0x2000000; i+=2) {
				WRITE16LE(temp, (i >> 1) & 0xFFFF);
				temp++;
			}

			bios = (u8 *)calloc(1,0x4000);
			if(bios == NULL) {
				systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
					"BIOS");
				CPUCleanUp();
			}    
			internalRAM = (u8 *)calloc(1,0x8000);
			if(internalRAM == NULL) {
				systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
					"IRAM");
				CPUCleanUp();
			}    
			paletteRAM = (u8 *)calloc(1,0x400);
			if(paletteRAM == NULL) {
				systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
					"PRAM");
				CPUCleanUp();
			}      
			vram = (u8 *)calloc(1, 0x20000);
			if(vram == NULL) {
				systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
					"VRAM");
				CPUCleanUp();
			}      
			oam = (u8 *)calloc(1, 0x400);
			if(oam == NULL) {
				systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
					"OAM");
				CPUCleanUp();
			}      

			/*pix = (u8 *)calloc(1, 4 * 241 * 162);
			if(pix == NULL) {
			systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
			"PIX");
			CPUCleanUp();
			}      
			extern size_t gbaPitch;
			gbaPitch = 964;*/

			ioMem = (u8 *)calloc(1, 0x400);
			if(ioMem == NULL) {
				systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
					"IO");
				CPUCleanUp();
			}      

			memset(flashSaveMemory, 0xff, sizeof(flashSaveMemory));
			memset(eepromData, 255, sizeof(eepromData));

			extern void CPUUpdateRenderBuffers(bool);
			CPUUpdateRenderBuffers(true);

			ROMSize = read;
			romSize = read;

			if(data.ROM)
			{
				delete [] data.ROM;
			}

			EmulatorSettings ^settings = EmulatorSettings::Current;

			//setting for turbo skip
			extern int turboSkip;
			turboSkip = EmulatorSettings::Current->TurboFrameSkip;


			auto configs = settings->ROMConfigurations;

			if(configs != nullptr)
			{
				char buffer[5];
				strncpy_s(buffer, (const char *) &rom[0xac], 4);
				buffer[4] = 0;

				string codeA = string(buffer);
				String ^code = ref new String(wstring(codeA.begin(), codeA.end()).c_str());
				if(configs->HasKey(code))
				{
					ROMConfig config = configs->Lookup(code);
					if(config.flashSize != -1)
					{
						flashSetSize(config.flashSize);
					}
					if(config.mirroringEnabled != -1)
					{
						doMirroring(config.mirroringEnabled != 0);
					}
					if(config.rtcEnabled != -1)
					{
						rtcEnable(config.rtcEnabled != 0);
					}
					if(config.saveType != -1)
					{
						cpuSaveType = config.saveType;
					}
				}
			}

			skipBios = true;
			

			EmulatorGame::emulator = GBASystem;

			soundInit();

			CPUInit(nullptr, false);
			CPUReset();

			//emulating = true;

			ROMFile = file;
			ROMFolder = folder;
			gbaROMLoaded = true;


		}).then([]()
		{
			return LoadSRAMAsync();
		}).then([emulator]()
		{
			if(cheatsTmp != nullptr)
			{
				LoadCheats(cheatsTmp);
				cheatsTmp = nullptr;
			}
			emulator->Unpause();
			emulator->Start();
			VirtualController::GetInstance()->UpdateFormat(VirtualController::GetInstance()->GetFormat());
		}).then([](task<void> t)
		{
			try
			{
				t.get();
			}catch(Platform::Exception ^ex)
			{
				if(ex->HResult != E_FAIL)
				{
					throw ex;
				}
			}
		});
	}

	void LoadCheatsOnROMLoad(Windows::Foundation::Collections::IVector<PhoneDirect3DXamlAppComponent::CheatData ^> ^cheats)
	{
		cheatsTmp = cheats;
	}

	void LoadCheats(Windows::Foundation::Collections::IVector<PhoneDirect3DXamlAppComponent::CheatData ^> ^cheats)
	{
		if (gbaROMLoaded)
			LoadCheatsGBA(cheats);
		else
			LoadCheatsGB(cheats);

		

	}


	void LoadCheatsGBA(Windows::Foundation::Collections::IVector<PhoneDirect3DXamlAppComponent::CheatData ^> ^cheats)
	{
		cheatsDeleteAll(EmulatorSettings::Current->RestoreOldCheatValues);

		cheatsEnabled = (cheats->Size > 0);

		for (int i = 0; i < cheats->Size; i++)
		{
			auto data = cheats->GetAt(i);
			if(!data->Enabled)
				continue;

			Platform::String ^code = data->CheatCode;
			Platform::String ^desc = data->Description;

			string codeString(code->Begin(), code->End());
			string descString(desc->Begin(), desc->End());

			if(code->Length() == 13)
			{
				// Code Breaker
				cheatsAddCBACode(codeString.c_str(), descString.c_str());
			}else if(code->Length() == 16)
			{
				//gameshark v1, 2
				cheatsAddGSACode(codeString.c_str(), descString.c_str(), false); 
			}
			else if(code->Length() == 17)
			{
				//gameshark v3
				//remove space				
				codeString = codeString.substr(0, 8) + codeString.substr(9, 8);

				cheatsAddGSACode(codeString.c_str(), descString.c_str(), true); 
			}
		}
	}



	void LoadCheatsGB(Windows::Foundation::Collections::IVector<PhoneDirect3DXamlAppComponent::CheatData ^> ^cheats)
	{
		gbCheatRemoveAll();

		cheatsEnabled = (cheats->Size > 0);

		for (int i = 0; i < cheats->Size; i++)
		{
			auto data = cheats->GetAt(i);
			if(!data->Enabled)
				continue;

			Platform::String ^code = data->CheatCode;
			Platform::String ^desc = data->Description;

			string codeString(code->Begin(), code->End());
			string descString(desc->Begin(), desc->End());

			if(code->Length() == 11 || code->Length() == 7)
			{
				// GameGenie
				gbAddGgCheat(codeString.c_str(), descString.c_str());
			}else if(code->Length() == 8)
			{
				// Gameshark
				gbAddGsCheat(codeString.c_str(), descString.c_str());
			}
		}
	}


	task<ROMData> GetROMBytesFromFileAsync(StorageFile ^file)
	{
		auto inputStream = make_shared<IInputStream ^>();
		auto openTask = create_task(file->OpenSequentialReadAsync());

		return openTask.then([=] (IInputStream ^stream)
		{ 
			*inputStream = stream;
			return file->GetBasicPropertiesAsync();
		}).then([=](BasicProperties ^properties)
		{
			Buffer ^buffer = ref new Buffer(properties->Size);
			return (*inputStream)->ReadAsync(buffer, properties->Size, InputStreamOptions::None);
		}).then([=](IBuffer ^buffer)
		{			
			DataReader ^reader = DataReader::FromBuffer(buffer);
			Array<BYTE> ^bytes = ref new Array<BYTE>(buffer->Length);
			reader->ReadBytes(bytes);
			BYTE *rawBytes = new BYTE[buffer->Length];
			for (int i = 0; i < buffer->Length; i++)
			{
				rawBytes[i] = bytes[i]; 
			}

			ROMData data;
			data.Length = buffer->Length;
			data.ROM = rawBytes;

			return data;
		});
	}



}