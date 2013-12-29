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

		property int PadCenterXP
		{
			int get() { return this->padCenterXP; }
			void set(int value) 
			{ 
				this->padCenterXP = value; 
				//if(this->SettingsChanged)
				//{
				//	this->SettingsChanged();
				//}
			}
		}

		property int PadCenterYP
		{
			int get() { return this->padCenterYP; }
			void set(int value) 
			{ 
				this->padCenterYP = value; 
	/*			if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}

		property int ALeftP
		{
			int get() { return this->aLeftP; }
			void set(int value) 
			{ 
				this->aLeftP = value; 
			/*	if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}

		property int ATopP
		{
			int get() { return this->aTopP; }
			void set(int value) 
			{ 
				this->aTopP = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}


		property int BLeftP
		{
			int get() { return this->bLeftP; }
			void set(int value) 
			{ 
				this->bLeftP = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}

		property int BTopP
		{
			int get() { return this->bTopP; }
			void set(int value) 
			{ 
				this->bTopP = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}


		property int StartLeftP
		{
			int get() { return this->startLeftP; }
			void set(int value) 
			{ 
				this->startLeftP = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}

		property int StartTopP
		{
			int get() { return this->startTopP; }
			void set(int value) 
			{ 
				this->startTopP = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}


		property int SelectRightP
		{
			int get() { return this->selectRightP; }
			void set(int value) 
			{ 
				this->selectRightP = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}

		property int SelectTopP
		{
			int get() { return this->selectTopP; }
			void set(int value) 
			{ 
				this->selectTopP = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}


		property int LLeftP
		{
			int get() { return this->lLeftP; }
			void set(int value) 
			{ 
				this->lLeftP = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}

		property int LTopP
		{
			int get() { return this->lTopP; }
			void set(int value) 
			{ 
				this->lTopP = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}


		property int RRightP
		{
			int get() { return this->rRightP; }
			void set(int value) 
			{ 
				this->rRightP = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}

		property int RTopP
		{
			int get() { return this->rTopP; }
			void set(int value) 
			{ 
				this->rTopP = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}


		property int PadCenterXL
		{
			int get() { return this->padCenterXL; }
			void set(int value) 
			{ 
				this->padCenterXL = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}

		property int PadCenterYL
		{
			int get() { return this->padCenterYL; }
			void set(int value) 
			{ 
				this->padCenterYL = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}

		property int ALeftL
		{
			int get() { return this->aLeftL; }
			void set(int value) 
			{ 
				this->aLeftL = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}

		property int ATopL
		{
			int get() { return this->aTopL; }
			void set(int value) 
			{ 
				this->aTopL = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}


		property int BLeftL
		{
			int get() { return this->bLeftL; }
			void set(int value) 
			{ 
				this->bLeftL = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}

		property int BTopL
		{
			int get() { return this->bTopL; }
			void set(int value) 
			{ 
				this->bTopL = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}


		property int StartLeftL
		{
			int get() { return this->startLeftL; }
			void set(int value) 
			{ 
				this->startLeftL = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}

		property int StartTopL
		{
			int get() { return this->startTopL; }
			void set(int value) 
			{ 
				this->startTopL = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}


		property int SelectRightL
		{
			int get() { return this->selectRightL; }
			void set(int value) 
			{ 
				this->selectRightL = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}

		property int SelectTopL
		{
			int get() { return this->selectTopL; }
			void set(int value) 
			{ 
				this->selectTopL = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}


		property int LLeftL
		{
			int get() { return this->lLeftL; }
			void set(int value) 
			{ 
				this->lLeftL = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}

		property int LTopL
		{
			int get() { return this->lTopL; }
			void set(int value) 
			{ 
				this->lTopL = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}


		property int RRightL
		{
			int get() { return this->rRightL; }
			void set(int value) 
			{ 
				this->rRightL = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
			}
		}

		property int RTopL
		{
			int get() { return this->rTopL; }
			void set(int value) 
			{ 
				this->rTopL = value; 
				/*if(this->SettingsChanged)
				{
					this->SettingsChanged();
				}*/
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

		int padCenterXP;
		int padCenterYP;
		int aLeftP;
		int aTopP;
		int bLeftP;
		int bTopP;
		int startLeftP;
		int startTopP;
		int selectRightP;
		int selectTopP;
		int lLeftP;
		int lTopP;
		int rRightP;
		int rTopP;



		int padCenterXL;
		int padCenterYL;
		int aLeftL;
		int aTopL;
		int bLeftL;
		int bTopL;
		int startLeftL;
		int startTopL;
		int selectRightL;
		int selectTopL;
		int lLeftL;
		int lTopL;
		int rRightL;
		int rTopL;


		
	};
}

