#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")


//入力
class Input {
public:
	//namespace省略
	template <class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
	
public:
	//初期化
	void Initialize(HINSTANCE hInstance,HWND hwnd);
	//更新
	void update();

private:
	IDirectInputDevice8* keyboard = nullptr;
	IDirectInput8* directInput = nullptr;
	
};
