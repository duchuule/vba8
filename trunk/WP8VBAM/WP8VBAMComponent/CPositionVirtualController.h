#pragma once

#include <D3D11.h>
#include "defines.h"
#include <collection.h>
#include "VirtualController.h"
#include <math.h>

using namespace Platform;
using namespace Windows::UI::Input;


namespace Emulator
{



	struct PointerInfo
	{
		PointerPoint^ point;
		bool IsMoved;
		Platform::String^ description;
	};


	class CPositionVirtualController : public VirtualController
	{
	public:

		CPositionVirtualController(void);
		~CPositionVirtualController(void);

		void CPositionVirtualController::SetControllerPosition(Windows::Foundation::Collections::IVector<int>^ cpos);
		void CPositionVirtualController::GetControllerPosition(Windows::Foundation::Collections::IVector<int>^ ret);


		virtual void PointerPressed(PointerPoint ^point) override;
		virtual void PointerMoved(PointerPoint ^point) override;
		virtual void PointerReleased(PointerPoint ^point) override;

		virtual const Emulator::ControllerState *GetControllerState(void) override;

		std::map<unsigned int, PointerInfo*> *pointerInfos;

	private:
		static CPositionVirtualController *singleton;


	
	};
}