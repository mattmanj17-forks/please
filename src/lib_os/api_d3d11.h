#ifndef OS_API_D3D11_H
#define OS_API_D3D11_H

#ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#endif

#ifndef COBJMACROS
#   define COBJMACROS
#endif

#pragma push_macro("far")
#pragma push_macro("near")

#define far
#define near

#include <d3d11.h>

struct OS_D3d11Api
{
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swapchain;
	ID3D11RenderTargetView* target;
};

#pragma pop_macro("far")
#pragma pop_macro("near")

#endif //OS_API_D3D11_H
