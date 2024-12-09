#pragma once
#include <Windows.h>
#include <wrl.h>
#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定
#include <dinput.h>
#include "WinApp.h"

class Input
{

public:

	template <class T>using ComPtr = Microsoft::WRL::ComPtr<T>;

	//初期化
	void Initialize(WinApp* winApp);
	//更新
	void Update();

	//キーの状態
	bool PushKey(BYTE keyNumber);
	bool TriggerKey(BYTE keyNumber);

private:
	BYTE key[256] = {};
	BYTE preKey[256] = {};
	ComPtr<IDirectInputDevice8>keyboard;
	ComPtr<IDirectInput8>directInput = nullptr;
	WinApp* winApp_ = nullptr;
};