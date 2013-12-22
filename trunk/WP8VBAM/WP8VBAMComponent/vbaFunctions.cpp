#include "vbam/System.h"
#include "EmulatorSettings.h"
#include "VirtualController.h"
#include "WP8VBAMComponent.h"

using namespace Emulator;
using namespace PhoneDirect3DXamlAppComponent;

bool enableTurboMode = false;

void log(const char *,...) { }

void winSignal(int, int) { }

void winOutput(const char *s, u32 addr) { }

void (*dbgSignal)(int,int) = winSignal;
void (*dbgOutput)(const char *, u32) = winOutput;

bool systemPauseOnFrame() { return false; }
void systemGbPrint(u8 *,int,int,int,int) { }
void systemScreenCapture(int) { }
// updates the joystick data
bool systemReadJoypads() { return true; }
u32 systemGetClock() { return (u32) GetTickCount64(); }
void systemMessage(int, const char *, ...) { }
void systemSetTitle(const char *) { }
void systemWriteDataToSoundBuffer() { }
void systemSoundShutdown() { }
void systemSoundPause() { }
void systemSoundResume() { }
void systemSoundReset() { }
//SoundDriver *systemSoundInit() { return NULL; }
void systemScreenMessage(const char *) { }
void systemUpdateMotionSensor() { }
int  systemGetSensorX() { return 0; }
int  systemGetSensorY() { return 0; }
bool systemCanChangeSoundQuality() { return false; }
void systemShowSpeed(int){ }
void system10Frames(int){ }
void systemFrame(){ }
void systemGbBorderOn(){ }
void winlog(const char *, ...) { }
void systemOnWriteDataToSoundBuffer(const u16 * finalWave, int length) { }
void systemOnSoundShutdown() { }
extern SoundDriver *newXAudio2_Output();
extern void soundShutdown();
void systemGbPrint(unsigned char *, int, int, int, int, int) { }
Moga::Windows::Phone::ControllerManager^ GetMogaController(void);

SoundDriver * systemSoundInit()
{
	SoundDriver * drv = 0;
	soundShutdown();

	if(EmulatorSettings::Current->SoundEnabled)
	{	
		drv = newXAudio2_Output();
	}

	return drv;
}

u32 systemReadJoypad(int gamepad) 
{ 
	u32 res = 0;

	VirtualController *controller = VirtualController::GetInstance();
	if(!controller)
		return res;

	bool left = false;
	bool right = false;
	bool up = false;
	bool down = false;
	bool start = false;
	bool select = false;
	bool a = false;
	bool b = false;
	bool l = false;
	bool r = false;

	//Moga
	using namespace Moga::Windows::Phone;
	Moga::Windows::Phone::ControllerManager^ ctrl = Direct3DBackground::getController();
	if(EmulatorSettings::Current->UseMogaController && ctrl != nullptr && ctrl->GetState(Moga::Windows::Phone::ControllerState::Connection) == ControllerResult::Connected)
	{
		//pro only, the pocket version has the left joy stick output to both Axis and directional Keycode.
		if (ctrl->GetState(Moga::Windows::Phone::ControllerState::SelectedVersion) == ControllerResult::VersionMoga ) 
		{
			float axis_x = ctrl->GetAxisValue(Axis::X);
			float axis_y = ctrl->GetAxisValue(Axis::Y);

			float angle = 0;
			if (axis_x == 0)
			{
				if (axis_y > 0)
					angle = 1.571f;
				else if (axis_y < 0)
					angle = 4.712f;
				else
					angle = 1000.0f; //non existent value
			}
			else if (axis_x > 0)
			{
				angle = atan(axis_y / axis_x);

				if (angle < 0)
					angle += 6.283f;
			}
			else if (axis_x <0)
			{
				angle = atan(axis_y / axis_x) +  3.142f;
			}

			//convert to degree
			angle = angle / 3.142f * 180.0f;
		
			if ( angle <= 22.5f)
			{
				right = true;
			}
			else if ( angle <= 67.5f)
			{
				right = true;
				up = true;
			}
			else if (angle <= 112.5f)
			{
				up = true;
			}
			else if (  angle <= 157.5f)
			{
				up = true;
				left = true;
			}
			else if (angle <= 202.5f)
			{
				left = true;
			}
			else if (angle <= 247.5f)
			{
				left = true;
				down = true;
			}
			else if (angle <= 292.5f)
			{
				down = true;
			}
			else if (angle <= 337.5f)
			{
				down = true;
				right = true;
			}
			else if (angle <= 360.0f)
			{
				right = true;
			}
		}

		
		if(ctrl->GetKeyCode(KeyCode::A) == ControllerAction::Pressed || ctrl->GetKeyCode(KeyCode::X) == ControllerAction::Pressed)
		{
			b = true;
		}

		if(ctrl->GetKeyCode(KeyCode::B) == ControllerAction::Pressed || ctrl->GetKeyCode(KeyCode::Y) == ControllerAction::Pressed)
		{
			a = true;
		}

		if(ctrl->GetKeyCode(KeyCode::Start) == ControllerAction::Pressed)
		{
			start = true;
		}

		if(ctrl->GetKeyCode(KeyCode::Select) == ControllerAction::Pressed)
		{
			select = true;
		}

		if(ctrl->GetKeyCode(KeyCode::DirectionLeft) == ControllerAction::Pressed)
		{
			left = true;
		}

		if(ctrl->GetKeyCode(KeyCode::DirectionRight) == ControllerAction::Pressed)
		{
			right = true;
		}

		if(ctrl->GetKeyCode(KeyCode::DirectionUp) == ControllerAction::Pressed)
		{
			up = true;
		}

		if(ctrl->GetKeyCode(KeyCode::DirectionDown) == ControllerAction::Pressed)
		{
			down = true;
		}

		if(ctrl->GetKeyCode(KeyCode::L1) == ControllerAction::Pressed || ctrl->GetKeyCode(KeyCode::L2) == ControllerAction::Pressed)
		{
			l = true;
		}

		if(ctrl->GetKeyCode(KeyCode::R1) == ControllerAction::Pressed || ctrl->GetKeyCode(KeyCode::R2) == ControllerAction::Pressed)
		{
			r = true;
		}

	}

	const Emulator::ControllerState *state = controller->GetControllerState();


	if(state->APressed || a)
		res |= 1;
	if(state->BPressed || b)
		res |= 2;
	if(state->SelectPressed || select)
		res |= 4;
	if(state->StartPressed || start)
		res |= 8;
	if(state->RightPressed || right)
		res |= 16;
	if(state->LeftPressed || left)
		res |= 32;
	if(state->UpPressed || up)
		res |= 64;
	if(state->DownPressed || down)
		res |= 128;

	// disallow left + right or up + down of being pressed at the same time
	if((res & 48) == 48)
		res &= ~16;
	if((res & 192) == 192)
		res &= ~128;






	EmulatorSettings ^settings = EmulatorSettings::Current;
	
	if(settings->CameraButtonAssignment == 0)
	{
		if(enableTurboMode && !settings->IsTrial)
		{ 
			// Speed
			res |= 1024;
		}
		if(state->RPressed | r)
			res |= 256;
		if(state->LPressed | l)
			res |= 512;
	}else if(settings->CameraButtonAssignment == 1)
	{
		if(enableTurboMode)
		{ 
			// R Button
			res |= 256;
		}
		if((state->RPressed | r) && !settings->IsTrial)
			res |= 1024;
		if(state->LPressed | l)
			res |= 512;
	}else if(settings->CameraButtonAssignment == 2)
	{
		if(enableTurboMode)
		{ 
			// L Button
			res |= 512;
		}
		if(state->RPressed | r)
			res |= 256;
		if((state->LPressed | l) && !settings->IsTrial)
			res |= 1024;
	}else if(settings->CameraButtonAssignment == 3)
	{
		if(enableTurboMode)
		{ 
			// L + R Button
			res |= 512;
			res |= 256;
		}
		if((state->LPressed || state->RPressed || l || r) && !settings->IsTrial)
			res |= 1024;
	}

	return res;
}

int RGB_LOW_BITS_MASK = 65793;
int emulating;
bool systemSoundOn;
u16 systemColorMap16[0x10000];
u32 systemColorMap32[0x10000];
u16 systemGbPalette[24];
int systemRedShift;
int systemGreenShift;
int systemBlueShift;
int systemColorDepth;
int systemDebug;
int systemVerbose;
int systemFrameSkip;
int systemSaveUpdateCounter;