#include "ROMConfig.h"

using namespace Windows::Foundation::Collections;

#pragma once

#define ORIENTATION_LANDSCAPE			0
#define ORIENTATION_LANDSCAPE_RIGHT		1
#define ORIENTATION_PORTRAIT			2

extern bool synchronize;

namespace PhoneDirect3DXamlAppComponent
{
	public delegate void SettingsChangedDelegate(void);

	public enum class AspectRatioMode : int 
	{ 
		Original,
		Stretch, 
		One, 
		FourToThree,
		FiveToFour
	}; 

	public ref class EmulatorSettings sealed
	{
	public:
		static property int OrientationBoth
		{
			int get() { return 0; } 
		}

		static property int OrientationLandscape
		{
			int get() { return 1; }
		}

		static property int OrientationPortrait
		{
			int get() { return 2; }
		}

		static property EmulatorSettings ^Current
		{
			EmulatorSettings ^get() 
			{
				if(!instance)
				{
					instance = ref new EmulatorSettings();
				}
				return instance;
			}
		}


		property SettingsChangedDelegate ^SettingsChanged;

		property bool Initialized;
		property bool SoundEnabled
		{
			bool get() { return this->soundEnabled; }
			void set(bool value) 
			{ 
				this->soundEnabled = value; 
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		property bool UseMogaController
		{
			bool get() { return this->useMogaController; }
			void set(bool value) 
			{ 
				this->useMogaController = value; 
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}


		property bool VirtualControllerOnTop
		{
			bool get() { return this->vcontrollerOnTop; }
			void set(bool value) 
			{ 
				this->vcontrollerOnTop = value; 
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		property bool LowFrequencyMode
		{
			bool get() { return this->lowFreqMode; }
			void set(bool value) 
			{ 
				this->lowFreqMode = value;
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		/*property bool LowFrequencyModeMeasured
		{
			bool get() { return this->lowFreqModeMeasured; }
			void set(bool value) 
			{ 
				this->lowFreqModeMeasured = value;
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}*/

		property bool FullscreenStretch
		{
			bool get() { return this->fullscreenStretch; }
			void set(bool value)
			{
				this->fullscreenStretch = value;
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		property int ControllerScale
		{
			int get() { return this->controllerScale; }
			void set(int value)
			{
				this->controllerScale = value;
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		property int ButtonScale
		{
			int get() { return this->buttonScale; }
			void set(int value)
			{
				this->buttonScale = value;
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		property int Orientation
		{
			int get() { return this->orientation; }
			void set(int value) 
			{
				this->orientation = value;
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		property int ControllerOpacity
		{
			int get() { return this->controllerOpacity; }
			void set(int value) 
			{
				this->controllerOpacity = value;
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		property int FrameSkip
		{
			int get() { return this->frameSkip; }
			void set(int value) 
			{
				this->frameSkip = value;
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		property int TurboFrameSkip
		{
			int get() { return this->turboFrameSkip; }
			void set(int value) 
			{
				this->turboFrameSkip = value;
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		property int PowerFrameSkip
		{
			int get() { return this->powerFrameSkip; }
			void set(int value) 
			{
				this->powerFrameSkip = value;
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		property int ImageScaling
		{
			int get() { return this->imageScale; }
			void set(int value) 
			{
				this->imageScale = value;
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		property AspectRatioMode AspectRatio
		{
			AspectRatioMode get() { return this->aspect; }
			void set(AspectRatioMode value) 
			{
				this->aspect = value;
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		property int DPadStyle
		{
			int get() { return this->dpadStyle; }
			void set(int value) 
			{
				this->dpadStyle = value;
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		property int CameraButtonAssignment
		{
			int get() { return this->cameraAssignment; }
			void set(int value) 
			{
				this->cameraAssignment = value;
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		property float Deadzone
		{
			float get() { return this->deadzone; }
			void set(float value) 
			{
				this->deadzone = value;
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		property IMap<Platform::String ^, ROMConfig> ^ROMConfigurations
		{
			IMap<Platform::String ^, ROMConfig> ^get(void) { return this->romConfigs; }
			void set(IMap<Platform::String ^, ROMConfig> ^value) { this->romConfigs = value; }
		}

		property bool SynchronizeAudio
		{
			bool get(void) { return synchronize; }
			void set(bool value) { synchronize = value; }
		}

		property bool IsTrial
		{
			bool get(void) { return this->trial; }
			void set(bool value) { this->trial = value; }
		}

		property bool HideConfirmationDialogs
		{
			bool get(void) { return this->hideConfirmations; }
			void set(bool value) 
			{ 
				this->hideConfirmations = value; 
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		property bool HideLoadConfirmationDialogs
		{
			bool get(void) { return this->hideLoadConfirmations; }
			void set(bool value) 
			{ 
				this->hideLoadConfirmations = value; 
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		property bool AutoIncrementSavestates
		{
			bool get(void) { return this->autoIncSavestates; }
			void set(bool value) 
			{ 
				this->autoIncSavestates = value; 
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		property bool SelectLastState
		{
			bool get(void) { return this->selectLastState; }
			void set(bool value) 
			{ 
				this->selectLastState = value; 
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		property bool RestoreOldCheatValues
		{
			bool get(void) { return this->restoreOldCheatValues; }
			void set(bool value) 
			{ 
				this->restoreOldCheatValues = value; 
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		property bool ManualSnapshots
		{
			bool get(void) { return this->manualSnapshots; }
			void set(bool value) 
			{ 
				this->manualSnapshots = value; 
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		EmulatorSettings(void);
	private:
		bool soundEnabled;
		bool useMogaController;
		bool vcontrollerOnTop;
		bool lowFreqMode;
		//bool lowFreqModeMeasured;
		bool fullscreenStretch;
		int controllerScale; //scale of joystick
		int buttonScale; //scale of button
		int controllerOpacity;
		int orientation;
		int frameSkip;
		int imageScale;
		int turboFrameSkip;
		int powerFrameSkip;
		AspectRatioMode aspect;
		int dpadStyle;
		float deadzone;
		int cameraAssignment;
		bool trial;
		bool hideConfirmations;
		bool hideLoadConfirmations;
		bool autoIncSavestates;
		bool selectLastState;
		bool restoreOldCheatValues;
		bool manualSnapshots;
		IMap<Platform::String ^, ROMConfig> ^romConfigs;

		static EmulatorSettings ^instance;
	};
}

