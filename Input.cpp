#include <wrl.h>
#include <cassert>
#include "Input.h"

void Input::Initialize(HINSTANCE hInstance, HWND hwnd) {
	HRESULT result;  // 追加

	// DirectInputのインスタンス生成
	result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	//DirectInput初期化
	
	result = DirectInput8Create(
		hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	//キーボードデバイスの生成
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));

	//入力データの形式セット
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));

	// 排他/非排他レベルのセット
	result = keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));


}



void Input::update()
{
	//キーボード情報の取得開始
	keyboard->Acquire();
	//全キーの入力情報を取得する
	BYTE key[256] = {};
	keyboard->GetDeviceState(sizeof(key), key);
	if (key[DIK_0]) {
		OutputDebugStringA("Hit 0\n");
	}

}
