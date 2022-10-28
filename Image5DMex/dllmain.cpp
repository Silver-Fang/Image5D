﻿// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "哈希计算器.h"
#include "resource.h"
HMODULE 模块句柄;
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		哈希初始化();
		模块句柄 = hModule;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		哈希释放();
		break;
	}
	return TRUE;
}

