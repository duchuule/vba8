#pragma once 

namespace PhoneDirect3DXamlAppComponent
{
	public delegate void RequestAdditionalFrameHandler();
	public delegate void ContinueEmulationNotifier(void);
	public delegate void SnapshotCallback(const Platform::Array<unsigned char> ^pixelData, int pitch, Platform::String ^fileName);
	public delegate void SavestateCreatedCallback(int slot, Platform::String ^romFileName);
	public delegate void SavestateSelectedCallback(int newSlot, int oldSlot);
}