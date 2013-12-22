#include "pch.h"
#include "MogaController.h"


namespace Emulator
{

	MogaController *MogaController::singleton = nullptr;

	MogaController *MogaController::GetInstance(void)
	{
		return singleton;
	}

	MogaController::MogaController(void)
	{
		singleton = this;
	}

	MogaController::~MogaController(void)
	{
		
	}

	const ControllerState *MogaController::GetControllerState(void)
	{

	}
}