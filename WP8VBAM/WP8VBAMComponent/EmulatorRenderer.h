#pragma once

#include "Renderer.h"

// This class renders a simple spinning cube.
ref class EmulatorRenderer sealed : public Renderer
{
public:
	EmulatorRenderer();
	virtual ~EmulatorRenderer(void);

	// Direct3DBase methods.
	virtual void CreateDeviceResources() override;
	virtual void CreateWindowSizeDependentResources() override;
	virtual void Render() override;
	virtual void UpdateForWindowSizeChange(float width, float height) override;

	// Method for updating time-dependent objects.
	virtual void Update(float timeTotal, float timeDelta) override;

	void UpdateController(void);
	void ChangeOrientation(int orientation);

internal:
	void SetVirtualController(VirtualController *controller);
	void GetBackbufferData(uint8 **backbufferPtr, size_t *pitch, int *imageWidth, int *imageHeight);

private:
	VirtualController					*controller;
	void *MapBuffer(int index, size_t *rowPitch);
};
