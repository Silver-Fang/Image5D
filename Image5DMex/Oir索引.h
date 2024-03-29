#pragma once
#include <stdint.h>
#include "哈希计算器.h"
struct 设备颜色
{
	char 设备名称[8];
	float 红;
	float 绿;
	float 蓝;
	float 不透明度;
};
class Oir索引
{
	//依赖字段

	UCHAR SHA256[哈希长度];
public:
	//独立字段

	UINT16 SizeX;
	UINT16 SizeY;
	UINT8 SizeC;
	UINT8 SizeZ;
	UINT8 每帧分块数;
	float 系列间隔;
	//依赖字段

	UINT8 SizeBC;
	UINT8 SizeZBC;
	UINT16 SizeT;
	uint32_t SizeTZBC;
	UINT32 SizeYX;
	UINT32 SizeCYX;
	bool 验证(uint32_t 文件大小)const noexcept;
	//需要正确设置每帧分块数和SizeC
	size_t 计算文件大小()const noexcept { return (char*)((设备颜色*)((UINT32*)(this + 1) + 每帧分块数) + SizeC) - (char*)this; }
	//需要正确设置每帧分块数和SizeC
	void Get变长成员(UINT32*& 每块像素数, 设备颜色*& 通道颜色, UINT64*& 块偏移)const noexcept
	{
		每块像素数 = (UINT32*)(this + 1);
		通道颜色 = (设备颜色*)(每块像素数 + 每帧分块数);
		块偏移 = (UINT64*)(通道颜色 + SizeC);
	}
	//需要正确设置所有独立字段
	void 计算依赖字段(uint32_t 块总数)noexcept;
	//需要正确设置所有字段
	void 哈希签名(uint32_t 文件大小)noexcept 
	{
		哈希计算(this + 哈希长度, 文件大小 - 哈希长度, SHA256);
	}
};