#pragma once



namespace Emulator
{

	struct ControllerState
	{
		bool LeftPressed;
		bool UpPressed;
		bool RightPressed;
		bool DownPressed;
		bool StartPressed;
		bool SelectPressed;
		bool APressed;
		bool BPressed;
		bool LPressed;
		bool RPressed;
	};

	class MogaController
	{
	public:
		static MogaController *GetInstance(void);

		MogaController(void);
		~MogaController(void);

		const ControllerState *GetControllerState(void);

	private:
		static Moga::Windows::Phone::ControllerManager^ singleton;
		ControllerState state;

	};
}