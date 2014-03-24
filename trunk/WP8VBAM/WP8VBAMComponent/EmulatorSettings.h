#include "ROMConfig.h"

using namespace Windows::Foundation::Collections;

#pragma once

#define ORIENTATION_LANDSCAPE			0
#define ORIENTATION_LANDSCAPE_RIGHT		1
#define ORIENTATION_PORTRAIT			2


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


		//property bool VirtualControllerOnTop
		//{
		//	bool get() { return this->vcontrollerOnTop; }
		//	void set(bool value) 
		//	{ 
		//		this->vcontrollerOnTop = value; 
		//		if(this->SettingsChanged)
		//		{
		//			this->SettingsChanged();
		//		}
		//	}
		//}

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
			int get() { return 0; } //set this to 0 because this value is useless
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
			bool get(void) { return this->synchronizeAudio; }
			void set(bool value) 
			{ 
				this->synchronizeAudio = value; 

				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
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




		property int BgcolorR
		{
			int get(void) { return this->bgcolorR; }
			void set(int value) 
			{ 
				this->bgcolorR = value; 
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		property int BgcolorG
		{
			int get(void) { return this->bgcolorG; }
			void set(int value) 
			{ 
				this->bgcolorG = value; 
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}


		property int BgcolorB
		{
			int get(void) { return this->bgcolorB; }
			void set(int value) 
			{ 
				this->bgcolorB = value; 
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}

		property bool AutoSaveLoad
		{
			bool get(void) { return this->autoSaveLoad; }
			void set(bool value) 
			{ 
				this->autoSaveLoad = value; 
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}


		property int VirtualControllerStyle
		{
			int get(void) { return this->virtualControllerStyle; }
			void set(int value) 
			{ 
				this->virtualControllerStyle = value; 
				if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}
			}
		}


		EmulatorSettings(void);

		property int PadCenterXP;
		property int PadCenterYP;
		property int ALeftP;
		property int ATopP;
		property int BLeftP;
		property int BTopP;
		property int StartLeftP;
		property int StartTopP;
		property int SelectRightP;
		property int SelectTopP;
		property int LLeftP;
		property int LTopP;
		property int RRightP;
		property int RTopP;



		property int PadCenterXL;
		property int PadCenterYL;
		property int ALeftL;
		property int ATopL;
		property int BLeftL;
		property int BTopL;
		property int StartLeftL;
		property int StartTopL;
		property int SelectRightL;
		property int SelectTopL;
		property int LLeftL;
		property int LTopL;
		property int RRightL;
		property int RTopL;

		property int MogaA;
		property int MogaB;
		property int MogaX;
		property int MogaY;
		property int MogaL1;
		property int MogaR1;
		property int MogaL2;
		property int MogaR2;
		property int MogaLeftJoystick;
		property int MogaRightJoystick;


	private:
		bool soundEnabled;
		bool synchronizeAudio;
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
		int bgcolorR;
		int bgcolorG;
		int bgcolorB;
		bool autoSaveLoad;
		int virtualControllerStyle;


		IMap<Platform::String ^, ROMConfig> ^romConfigs;

		static EmulatorSettings ^instance;




		
	};
}

