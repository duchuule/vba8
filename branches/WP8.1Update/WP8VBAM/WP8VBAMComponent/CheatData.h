#ifndef CHEATDATA_H_
#define CHEATDATA_H_

namespace PhoneDirect3DXamlAppComponent
{
	public ref class CheatData sealed
	{
	public:
		property Platform::String ^CheatCode;
		property Platform::String ^Description;
		property bool Enabled;
	};
}

#endif