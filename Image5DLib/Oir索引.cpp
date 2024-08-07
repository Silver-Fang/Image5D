#include "Oir索引.h"
#include"Image5D异常.h"
#include<string.h>
using namespace Image5D;
bool Oir索引::验证(uint32_t 文件大小)const
{
	if (文件版本号 < 库版本号)
		return false;
	else if (文件版本号 > 库版本号)
		throw Exception::The_library_version_is_lower_than_the_index_file_version;
	char 哈希值[哈希长度];
	哈希计算((char*)this + 哈希长度, 文件大小 - 哈希长度, 哈希值);
	return !memcmp(哈希值, SHA256, 哈希长度);
}
void Oir索引::计算依赖字段(uint32_t 块总数)noexcept
{
	SizeBC = 每帧分块数 * SizeC;
	const uint32_t SizeTZ = 块总数 / SizeBC;
	if (SizeTZ < SizeZ)
		SizeZ = SizeTZ;
	SizeT = SizeTZ / SizeZ;
	SizeZBC = (uint16_t)SizeZ * SizeBC;
	SizeTZBC = SizeT * SizeZBC;
	SizeYX = (uint32_t)SizeY * SizeX;
	SizeCYX = SizeC * SizeYX;
	SizeZC = SizeZ * SizeC;
}