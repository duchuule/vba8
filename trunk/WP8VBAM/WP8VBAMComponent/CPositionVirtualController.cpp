#include "pch.h"
#include "CPositionVirtualController.h"
#include "EmulatorSettings.h"
#include <string>
#include <xstring>
#include <sstream>

using namespace std;

using namespace PhoneDirect3DXamlAppComponent;
	
#define VCONTROLLER_Y_OFFSET_WVGA				218
#define VCONTROLLER_Y_OFFSET_WXGA				348
#define VCONTROLLER_Y_OFFSET_720P				308
#define VCONTROLLER_BUTTON_Y_OFFSET_WVGA			303
#define VCONTROLLER_BUTTON_Y_OFFSET_WXGA			498
#define VCONTROLLER_BUTTON_Y_OFFSET_720P			428

namespace Emulator
{


	CPositionVirtualController *CPositionVirtualController::singleton = nullptr;


	CPositionVirtualController::CPositionVirtualController(void)
	{
		InitializeCriticalSectionEx(&this->cs, 0, 0);
		virtualControllerOnTop = false;
		this->pointerInfos = new std::map<unsigned int, PointerInfo*>();
		singleton = this;
	}

	CPositionVirtualController::~CPositionVirtualController(void)
	{
		DeleteCriticalSection(&this->cs);
	}
	

	void CPositionVirtualController::PointerPressed(PointerPoint ^point)
	{
		EnterCriticalSection(&this->cs);
		PointerInfo* pinfo = new PointerInfo();
		pinfo->point = point;
		pinfo->description = "";
		pinfo->IsMoved = false;

		this->pointerInfos->insert(std::pair<unsigned int, PointerInfo*>(point->PointerId, pinfo));
		

		LeaveCriticalSection(&this->cs);
		
	}

	void CPositionVirtualController::PointerMoved(PointerPoint ^point)
	{
		EnterCriticalSection(&this->cs);

		std::map<unsigned int, PointerInfo*>::iterator iter = this->pointerInfos->find(point->PointerId);

		if(iter != this->pointerInfos->end())
		{
			PointerInfo* pinfo = iter->second;
			pinfo->IsMoved = true;
			pinfo->point = point;
		}
		

		LeaveCriticalSection(&this->cs);
	}

	void CPositionVirtualController::PointerReleased(PointerPoint ^point)
	{
		EnterCriticalSection(&this->cs);
		std::map<unsigned int, PointerInfo*>::iterator iter = this->pointerInfos->find(point->PointerId);
		if(iter != this->pointerInfos->end())
		{
			this->pointerInfos->erase(iter);
		}

		

		LeaveCriticalSection(&this->cs);
	}
	
	const Emulator::ControllerState *CPositionVirtualController::GetControllerState(void)
	{
		ZeroMemory(&this->state, sizeof(ControllerState));

		int dpad = EmulatorSettings::Current->DPadStyle;

		EnterCriticalSection(&this->cs);
		//typedef std::map<unsigned int, PointerInfo*>::iterator it_type;




		auto i = this->pointerInfos->begin();
		if (i != this->pointerInfos->end()) //we only read in one point
		{
			PointerPoint ^p = i->second->point;
			Windows::Foundation::Point point = Windows::Foundation::Point(p->Position.Y, p->Position.X);

			if (this->stickBoundaries.Contains(p->Position))
			{
				i->second->description = "joystick";
				state.JoystickPressed = true;
			}

		
			if(this->startRect.Contains(point))
			{
				state.StartPressed = true;
				i->second->description = "start";

			}
			if(this->selectRect.Contains(point))
			{
				state.SelectPressed = true;
				i->second->description = "select";

			}
			if(this->lRect.Contains(point))
			{
				state.LPressed = true;
				i->second->description = "l";

			}
			if(this->rRect.Contains(point))
			{
				state.RPressed = true;
				i->second->description = "r";

			}
			if(this->aRect.Contains(point))
			{
				state.APressed = true;
				i->second->description = "a";

			}
			if(this->bRect.Contains(point))
			{
				state.BPressed = true;
				i->second->description = "b";
			}
		}
		LeaveCriticalSection(&this->cs);

		return &this->state;
	}




	
	
}