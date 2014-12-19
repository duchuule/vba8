#pragma once

#include <ppltasks.h>
#include "Emulator.h"
#include "CheatData.h"

using namespace concurrency;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;

#define MAX_SAVESTATE_SLOTS 10
#define AUTOSAVE_SLOT		9

namespace Emulator
{
	extern int ROMSize;
	extern StorageFile ^ROMFile;
	extern StorageFolder ^ROMFolder;
	extern int SavestateSlot;
	extern int LoadstateSlot;
	extern int ROMSize2;
	extern StorageFile ^ROMFile2;
	extern StorageFolder ^ROMFolder2;
	extern int SavestateSlot2;
	extern int LoadstateSlot2;

	Platform::Array<unsigned char> ^GetSnapshotBuffer(unsigned char *backbuffer, size_t pitch, int imageWidth, int imageHeight);
	task<void> SaveStateAsync(void);
	task<void> SaveGBAStateAsync(void);
	task<void> SaveGBStateAsync(void);
	//task<void> LoadStateAsync(void);
	task<void> LoadStateAsync(int slot);
	//task<void> LoadGBAStateAsync(void);
	task<void> LoadGBAStateAsync(int slot);
	//task<void> LoadGBStateAsync(void);
	task<void> LoadGBStateAsync(int slot);

	task<void> LoadROMAsync(StorageFile ^file, StorageFolder ^folder);
	task<void> LoadGBROMAsync(StorageFile ^file, StorageFolder ^folder);
	task<void> LoadGBAROMAsync(StorageFile ^file, StorageFolder ^folder);
	task<void> ResetAsync(void);
	void ResetSync(void);
	task<ROMData> GetROMBytesFromFileAsync(StorageFile ^file);
	task<void> SaveSRAMAsync(void);
	task<void> SaveGBSRAMAsync(void);
	task<void> SaveGBASRAMAsync(void);
	task<void> LoadSRAMAsync(void);
	task<void> LoadGBASRAMAsync(void);
	task<void> LoadGBSRAMAsync(void);
	void LoadCheatsOnROMLoad(Windows::Foundation::Collections::IVector<PhoneDirect3DXamlAppComponent::CheatData ^> ^cheats);
	void LoadCheats(Windows::Foundation::Collections::IVector<PhoneDirect3DXamlAppComponent::CheatData ^> ^cheats);
	void LoadCheatsGBA(Windows::Foundation::Collections::IVector<PhoneDirect3DXamlAppComponent::CheatData ^> ^cheats);
	void LoadCheatsGB(Windows::Foundation::Collections::IVector<PhoneDirect3DXamlAppComponent::CheatData ^> ^cheats);

	task<void> SaveBytesToFileAsync(StorageFile ^file, unsigned char *bytes, size_t length);
	task<ROMData> GetBytesFromFileAsync(StorageFile ^file);
}