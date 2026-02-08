#pragma once

struct RenderTarget {
	bool cameraEnabled = true;
	bool rotationEnabled = false;
	bool renderTriangle = false;
	bool renderMandelbulb = false;
};

inline RenderTarget g_renderTarget{};