#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include"WinApp.h"

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")


//入力
class Input {
public:
	//namespace省略
	template <class T>using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	//初期化
	void Initialize(WinApp* winApp);
	//更新
	void update();

	/// <summary>
	/// キーの押下をチェック
	/// </summary>
	/// 	キー番号
	/// 	おされているか
	bool PushKey(BYTE keyNumBer);
	bool TrrigerKey(BYTE keyNumBer);
private:

	IDirectInputDevice8* keyboard = nullptr;
	IDirectInput8* directInput = nullptr;

	WinApp* winApp = nullptr;
	//全キーの入力情報を取得する
	BYTE key[256] = {};
	BYTE prevKey[256] = {};

};
