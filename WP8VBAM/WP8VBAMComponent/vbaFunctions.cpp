#include "vbam/System.h"
#include "EmulatorSettings.h"
#include "VirtualController.h"
#include "WP8VBAMComponent.h"
#include <math.h>  

using namespace Emulator;
using namespace PhoneDirect3DXamlAppComponent;

extern bool synchronize;

bool cameraPressed = false;
bool autoFireToggle = false;
int mappedButton = 0;
int sensorX = 2047;
int sensorY = 2047;

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
void GetMogaMapping(int pressedButton, bool* a, bool* b, bool* l, bool* r );
void GetMotionMapping(int tiltDirection, bool* left, bool* right, bool* up, bool* down, bool* a, bool* b, bool* l, bool* r);




SoundDriver * systemSoundInit()
{
	SoundDriver * drv = 0;
	soundShutdown();

	synchronize = EmulatorSettings::Current->SynchronizeAudio;

	if(EmulatorSettings::Current->SoundEnabled)
	{	
		drv = newXAudio2_Output();
	}

	return drv;
}

double SolveForAngle(double a, double b, double c)
{
	const double epsilon = 0.0000000000000001;
	double x = -1001.0, x2 = -1001.0;
	//solve the function A cos(x) + B sin(x) = C
	if (abs(a + c) < epsilon) //when a + c = 0
	{
		x = (-2 * atan(a / b)) / 3.14159265 * 180;
	}
	else
	{
		double check = a*a + b*b + a*c - b*sqrt(a*a + b*b - c*c);
		if (abs(check) > epsilon)
		{
			//2 ArcTan[(b - Sqrt[a ^ 2 + b ^ 2 - c ^ 2]) / (a + c)]
			x = (2 * atan((b - sqrt(a*a + b*b - c*c)) / (a + c) ) ) / 3.14159265 * 180;
		}

		check = a*a + b*b + a*c + b*sqrt(a*a + b*b - c*c);
		if (abs(check > epsilon))
		{
			//x == 2 ArcTan[(b + Sqrt[a^2 + b^2 - c^2])/(a + c)]
			x2 = (2 * atan((b + sqrt(a*a + b*b - c*c)) / (a + c))) / 3.14159265 * 180;
		}
	}

	if (x < -1000 && x2 > -1000)
		x = x2;

	if (x > -1000 && x2 > -1000) //2 solutions, choose the most likely one
	{
		if (abs(x2) < abs(x))
			x = x2;
	}

	return x;
}

double deg2rad(double deg)
{
	return deg / 180.0 * 3.14159265;
}

u8 getMotionInput()
{
	//bit order: left: 1, right: 2, up: 4, down: 8
	int ret = 0;


	VirtualController *controller = VirtualController::GetInstance();
	if (!controller)
		return ret;

	bool left = false;
	bool right = false;
	bool up = false;
	bool down = false;
	
	EmulatorSettings ^settings = EmulatorSettings::Current;

	Windows::Devices::Sensors::Accelerometer^ accl = Direct3DBackground::getAccelormeter();
	Windows::Devices::Sensors::Inclinometer^ incl = Direct3DBackground::getInclinometer();

	//Motion Control
	//see Tilt Sensing Using a Three-Axis Accelerometer for complete equations
	//[Gx, Gy, Gz]^T = Rx(phi) Ry(theta) [Gx0, Gy0, Gz0]^T
	// Combined rotation matrix: [Gx0*cos(theta) - Gz0*sin(theta), Gx0*sin(theta)*sin(phi)+Gy0*cos(phi) + cos(theta)*sin(phi)*Gz0,
	//							Gx0*cos(phi)*sin(theta) + Gy0*(-sin(phi)) + cos(theta)*cos(phi)*Gz0 ]

	double rotationDeadZone = 10;
	double g0[3] = { settings->RestAngleX, settings->RestAngleY, settings->RestAngleZ };

	//correct for orientation if needed
	if (settings->MotionAdaptOrientation)
	{
		if (settings->UseMotionControl == 1)
		{
			if (controller->GetOrientation() == ORIENTATION_PORTRAIT)
			{
				if (abs(settings->RestAngleX) < abs(settings->RestAngleY)) //phone is calibrated in portrait, RestAngleX = 0, Y = -0.7
				{
					g0[0] = settings->RestAngleX;
					g0[1] = settings->RestAngleY;

				}
				else //phone is calibrated in landscape left
				{
					if (settings->RestAngleX < 0)  //calibrated in landscape left
					{
						g0[0] = settings->RestAngleY;
						g0[1] = settings->RestAngleX;
					}
					else  //calibrated in landscape right
					{
						g0[0] = -settings->RestAngleY;
						g0[1] = -settings->RestAngleX;
					}
				}
			}
			else if (controller->GetOrientation() == ORIENTATION_LANDSCAPE) //current orientation is landscape left
			{
				if (abs(settings->RestAngleX) < abs(settings->RestAngleY)) //phone is calibrated in portrait, RestAngleX = 0, Y = -0.7
				{
					g0[0] = settings->RestAngleY;
					g0[1] = -settings->RestAngleX;
				}
				else //phone is calibrated in landscape left
				{
					if (settings->RestAngleX < 0)  //calibrated in landscape left
					{
						g0[0] = settings->RestAngleX;
						g0[1] = settings->RestAngleY;
					}
					else  //calibrated in landscape right
					{
						g0[0] = -settings->RestAngleX;
						g0[1] = -settings->RestAngleY;
					}
				}
			}
			else //current orientation is landscape right
			{
				if (abs(settings->RestAngleX) < abs(settings->RestAngleY)) //phone is calibrated in portrait, RestAngleX = 0, Y = -0.7
				{
					g0[0] = -settings->RestAngleY;
					g0[1] = settings->RestAngleX;
				}
				else //phone is calibrated in landscape left
				{
					if (settings->RestAngleX < 0)  //calibrated in landscape left
					{
						g0[0] = -settings->RestAngleX;
						g0[1] = -settings->RestAngleY;
					}
					else  //calibrated in landscape right
					{
						g0[0] = settings->RestAngleX;
						g0[1] = settings->RestAngleY;
					}
				}
			}
		}
		else if (settings->UseMotionControl == 2) //inclinometer
		{
			if (controller->GetOrientation() == ORIENTATION_PORTRAIT) //RestAngleX = 0, Y = 45
			{
				if (abs(settings->RestAngleX) < abs(settings->RestAngleY)) //phone is calibrated in portrait, RestAngleX = 0, Y = 45
				{
					g0[0] = settings->RestAngleX;
					g0[1] = settings->RestAngleY;

				}
				else //phone is calibrated in landscape 
				{
					if (settings->RestAngleX < 0)  //calibrated in landscape left
					{
						g0[0] = settings->RestAngleY;
						g0[1] = -settings->RestAngleX;
					}
					else  //calibrated in landscape right
					{
						g0[0] = -settings->RestAngleY;
						g0[1] = settings->RestAngleX;
					}
				}
			}
			else if (controller->GetOrientation() == ORIENTATION_LANDSCAPE) //current orientation is landscape left, RestAngleX = -45, Y = 0
			{
				if (abs(settings->RestAngleX) < abs(settings->RestAngleY)) //phone is calibrated in portrait, RestAngleX = 0, Y = 45
				{
					g0[0] = -settings->RestAngleY;
					g0[1] = settings->RestAngleX;
				}
				else //phone is calibrated in landscape left
				{
					if (settings->RestAngleX < 0)  //calibrated in landscape left
					{
						g0[0] = settings->RestAngleX;
						g0[1] = settings->RestAngleY;
					}
					else  //calibrated in landscape right
					{
						g0[0] = -settings->RestAngleX;
						g0[1] = -settings->RestAngleY;
					}
				}
			}
			else //current orientation is landscape right, RestAngleX = 45, Y = 0
			{
				if (abs(settings->RestAngleX) < abs(settings->RestAngleY)) //phone is calibrated in portrait, RestAngleX = 0, Y = 45
				{
					g0[0] = settings->RestAngleY;
					g0[1] = -settings->RestAngleX;
				}
				else //phone is calibrated in landscape left
				{
					if (settings->RestAngleX < 0)  //calibrated in landscape left
					{
						g0[0] = -settings->RestAngleX;
						g0[1] = -settings->RestAngleY;
					}
					else  //calibrated in landscape right
					{
						g0[0] = settings->RestAngleX;
						g0[1] = settings->RestAngleY;
					}
				}
			}
		
		}

	}



	double g[3];

	//NOTE: x is phone's short edge, y is phone's long endge, z is phone's thickness
	if (settings->UseMotionControl == 1 && accl != nullptr)
	{
		Windows::Devices::Sensors::AccelerometerReading^ reading = accl->GetCurrentReading();

		g[0] = reading->AccelerationX;
		g[1] = reading->AccelerationY;
		g[2] = reading->AccelerationZ;

		double theta = SolveForAngle(g0[0], -g0[2], g[0]);



		double phi = SolveForAngle(g0[1], g0[0] * sin(deg2rad(theta)) + g0[2] * cos(deg2rad(theta)), g[1]);

		//account for different orientation

		if (controller->GetOrientation() == ORIENTATION_PORTRAIT)
		{

			if (theta < -settings->MotionDeadzoneH)
				left = true;
			else if (theta > settings->MotionDeadzoneH)
				right = true;


			if (phi < -settings->MotionDeadzoneV)
				up = true;
			else if (phi > settings->MotionDeadzoneV)
				down = true;
		}
		else if (controller->GetOrientation() == ORIENTATION_LANDSCAPE)
		{
			if (theta < -settings->MotionDeadzoneH)
				down = true;
			else if (theta > settings->MotionDeadzoneH)
				up = true;


			if (phi < -settings->MotionDeadzoneV)
				left = true;
			else if (phi > settings->MotionDeadzoneV)
				right = true;

		}
		else
		{
			if (theta < -settings->MotionDeadzoneH)
				up = true;
			else if (theta > settings->MotionDeadzoneH)
				down = true;


			if (phi < -settings->MotionDeadzoneV)
				right = true;
			else if (phi > settings->MotionDeadzoneV)
				left = true;
		}
	}
	else if (settings->UseMotionControl == 2 && incl != nullptr)
	{
		Windows::Devices::Sensors::InclinometerReading^ reading = incl->GetCurrentReading();


		//account for different orientation

		if (controller->GetOrientation() == ORIENTATION_PORTRAIT)
		{
			if (reading->RollDegrees - g0[0] < -settings->MotionDeadzoneH)
				left = true;
			else if (reading->RollDegrees - g0[0] > settings->MotionDeadzoneH)
				right = true;

			if (reading->PitchDegrees - g0[1] < -settings->MotionDeadzoneV)
				up = true;
			else if (reading->PitchDegrees - g0[1] > settings->MotionDeadzoneV)
				down = true;
		}
		else if (controller->GetOrientation() == ORIENTATION_LANDSCAPE)
		{
			if (reading->RollDegrees - g0[0] < -settings->MotionDeadzoneH)
				down = true;
			else if (reading->RollDegrees - g0[0] > settings->MotionDeadzoneH)
				up = true;

			if (reading->PitchDegrees - g0[1] < -settings->MotionDeadzoneV)
				left = true;
			else if (reading->PitchDegrees - g0[1] > settings->MotionDeadzoneV)
				right = true;

		}
		else
		{

			if (reading->RollDegrees - g0[0] < -settings->MotionDeadzoneH)
				up = true;
			else if (reading->RollDegrees - g0[0] > settings->MotionDeadzoneH)
				down = true;

			if (reading->PitchDegrees - g0[1] < -settings->MotionDeadzoneV)
				right = true;
			else if (reading->PitchDegrees - g0[1] > settings->MotionDeadzoneV)
				left = true;
		}

		
	}

	if (left)
		ret |= 1;
	if (right)
		ret |= 2;
	if (up)
		ret |= 4;
	if (down)
		ret |= 8;

	return ret;

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

	EmulatorSettings ^settings = EmulatorSettings::Current;

	

	u8 motionInput = getMotionInput();

	if (motionInput & 1)
		GetMotionMapping(settings->MotionLeft, &left, &right, &up, &down, &a, &b, &l, &r);
	else if (motionInput & 2)
		GetMotionMapping(settings->MotionRight, &left, &right, &up, &down, &a, &b, &l, &r);

	if (motionInput & 4)
		GetMotionMapping(settings->MotionUp, &left, &right, &up, &down, &a, &b, &l, &r);
	else if (motionInput & 8)
		GetMotionMapping(settings->MotionDown, &left, &right, &up, &down, &a, &b, &l, &r);


	//Moga
	using namespace Moga::Windows::Phone;
	Moga::Windows::Phone::ControllerManager^ ctrl = Direct3DBackground::getController();
	if(EmulatorSettings::Current->UseMogaController && ctrl != nullptr && ctrl->GetState(Moga::Windows::Phone::ControllerState::Connection) == ControllerResult::Connected)
	{
		//pro only, the pocket version has the left joy stick output to both Axis and directional Keycode.
		if (ctrl->GetState(Moga::Windows::Phone::ControllerState::SelectedVersion) != ControllerResult::VersionMoga ) 
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

		if(ctrl->GetKeyCode(KeyCode::A) == ControllerAction::Pressed )
		{
			GetMogaMapping(settings->MogaA, &a, &b, &l, &r );
		}

		
		if(ctrl->GetKeyCode(KeyCode::B) == ControllerAction::Pressed)
			GetMogaMapping(settings->MogaB, &a, &b, &l, &r );


		if (ctrl->GetKeyCode(KeyCode::X) == ControllerAction::Pressed)
			GetMogaMapping(settings->MogaX, &a, &b, &l, &r );


		if(ctrl->GetKeyCode(KeyCode::Y) == ControllerAction::Pressed)
			GetMogaMapping(settings->MogaY, &a, &b, &l, &r );

		if(ctrl->GetKeyCode(KeyCode::L1) == ControllerAction::Pressed )
			GetMogaMapping(settings->MogaL1, &a, &b, &l, &r );

		if (abs(ctrl->GetAxisValue(Axis::LeftTrigger)) > 0.5f)
			GetMogaMapping(settings->MogaL2, &a, &b, &l, &r );


		if(ctrl->GetKeyCode(KeyCode::R1) == ControllerAction::Pressed )
			GetMogaMapping(settings->MogaR1, &a, &b, &l, &r );


		if (abs(ctrl->GetAxisValue(Axis::RightTrigger)) > 0.5f)
			GetMogaMapping(settings->MogaR2, &a, &b, &l, &r );

		if(ctrl->GetKeyCode(KeyCode::ThumbLeft) == ControllerAction::Pressed )
			GetMogaMapping(settings->MogaLeftJoystick, &a, &b, &l, &r );

		if(ctrl->GetKeyCode(KeyCode::ThumbRight) == ControllerAction::Pressed )
			GetMogaMapping(settings->MogaRightJoystick, &a, &b, &l, &r );

	}

	try
	{
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
		if(state->RPressed || r)
			res |= 256;
		if(state->LPressed || l)
			res |= 512;

		// disallow left + right or up + down of being pressed at the same time
		if((res & 48) == 48)
			res &= ~16;
		if((res & 192) == 192)
			res &= ~128;


		//set the value for autoFire key depending on the setting
		if(settings->CameraButtonAssignment == 1) //R button
			mappedButton = 256;
		else if (settings->CameraButtonAssignment == 2) //L button
			mappedButton = 512;
		else if (settings->CameraButtonAssignment == 3) //A button
			mappedButton = 1;
		else if (settings->CameraButtonAssignment == 4) //B button
			mappedButton = 2;
		else
			mappedButton = 0;


		if (settings->CameraButtonAssignment != 0 && settings->MapABLRTurbo && (res & mappedButton) == mappedButton) //if this is true, the onscreen A/B/L/R need to be replace with turbo function
		{
			//first get rid of all previous value for the mapped button
			res &= (~mappedButton);

			//turbo speed
			res |= 1024;
		}

		if (settings->UseTurbo)
		{
			// Speed
			res |= 1024;

		}


		if (cameraPressed) //this is true when camera button is pressed
		{
			if (settings->EnableAutoFire)
			{
				res &= (~mappedButton);
				if (autoFireToggle)
					res |= mappedButton;
				autoFireToggle = !autoFireToggle;
				
			}
			else //no autofire, just keep pressing the button
			{
				res |= mappedButton;
			}
			
		}
	}
	catch (exception ex) 
	{}
	return res;
}

void GetMogaMapping(int pressedButton, bool* a, bool* b, bool* l, bool* r )
{
	if (pressedButton & 1)
		*a = true;
	if (pressedButton & 2)
		*b = true;
	if (pressedButton & 4) 
		*l = true;
	if (pressedButton & 8)
		*r = true;
	
}


void GetMotionMapping(int tiltDirection, bool* left, bool* right, bool* up, bool* down, bool* a, bool* b, bool* l, bool* r)
{
	if (tiltDirection & 1)
		*left = true;
	if (tiltDirection & 2)
		*right = true;
	if (tiltDirection & 4)
		*up = true;
	if (tiltDirection & 8)
		*down = true;
	if (tiltDirection & 16)
		*a = true;
	if (tiltDirection & 32)
		*b = true;
	if (tiltDirection & 64)
		*l = true;
	if (tiltDirection & 128)
		*r = true;

}


void systemUpdateMotionSensor()
{
	u8 motionInput = getMotionInput();

	if (motionInput & 1) {
		sensorX += 3;
		if (sensorX > 2197)
			sensorX = 2197;
		if (sensorX < 2047)
			sensorX = 2057;
	}
	else if (motionInput & 2) {
		sensorX -= 3;
		if (sensorX < 1897)
			sensorX = 1897;
		if (sensorX > 2047)
			sensorX = 2037;
	}
	else if (sensorX > 2047) {
		sensorX -= 2;
		if (sensorX < 2047)
			sensorX = 2047;
	}
	else {
		sensorX += 2;
		if (sensorX > 2047)
			sensorX = 2047;
	}

	if (motionInput & 4) {
		sensorY += 3;
		if (sensorY > 2197)
			sensorY = 2197;
		if (sensorY < 2047)
			sensorY = 2057;
	}
	else if (motionInput & 8) {
		sensorY -= 3;
		if (sensorY < 1897)
			sensorY = 1897;
		if (sensorY > 2047)
			sensorY = 2037;
	}
	else if (sensorY > 2047) {
		sensorY -= 2;
		if (sensorY < 2047)
			sensorY = 2047;
	}
	else {
		sensorY += 2;
		if (sensorY > 2047)
			sensorY = 2047;
	}
}


int  systemGetSensorX() 
{
	return sensorX;
}
int  systemGetSensorY() 
{
	return sensorY;
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