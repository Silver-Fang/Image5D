#include <Image5D异常.h>
#include <Mex工具.h>
#include <Mex实现.h>
//OirReader
API声明(Oir_CreateReader);
API声明(Oir_DeleteReader);
API声明(Oir_SizeX);
API声明(Oir_SizeY);
API声明(Oir_SizeC);
API声明(Oir_SizeZ);
API声明(Oir_SizeT);
API声明(Oir_SeriesInterval);
API声明(Oir_DeviceColors);
API声明(Oir_ReadPixels);
API声明(Oir_ReadToPointer);

//OmeTiffRWer
API声明(Tiff_OpenRead);
API声明(Tiff_OpenRW);
API声明(Tiff_OpenCreate);
API声明(Tiff_PixelType);
API声明(Tiff_DimensionOrder);
API声明(Tiff_SizeP);
API声明(Tiff_SizeX);
API声明(Tiff_SizeY);
API声明(Tiff_SizeI);
API声明(Tiff_SizeC);
API声明(Tiff_SizeZ);
API声明(Tiff_SizeT);
API声明(Tiff_ChannelColors);
API声明(Tiff_FileName);
API声明(Tiff_ImageDescription);
API声明(Tiff_ModifyParameters);
API声明(Tiff_ReadPixels);
API声明(Tiff_ReadPixelsI);
API声明(Tiff_WritePixels);
API声明(Tiff_WritePixelsI);
API声明(Tiff_PixelPointer);
API声明(Tiff_PixelPointerI);
API声明(Tiff_ReadToPointer);
API声明(Tiff_ReadToPointerI);
API声明(Tiff_WriteFromPointer);
API声明(Tiff_WriteFromPointerI);
API声明(Tiff_Close);
using namespace Mex工具;
StructArray 异常转结构(const Image5D::Image5D异常& 异常)
{
	StructArray 返回 = 数组工厂.createStructArray({ 1 }, { "ExceptionType","InnerException","ErrorCode" });
	返回[0]["ExceptionType"] = 数组工厂.createScalar<uint8_t>(异常.异常类型);
	返回[0]["InnerException"] = 数组工厂.createScalar<uint8_t>(异常.内部异常);
	if (异常.内部异常 == Image5D::Image5D)
		返回[0]["ErrorCode"] = 异常转结构(*异常.内部Image5D异常);
	else
		返回[0]["ErrorCode"] = 数组工厂.createScalar(异常.错误代码);
	return 返回;
}
void MexFunction::operator()(ArgumentList& outputs, ArgumentList& inputs)
{
	API索引{
		// OirReader
		Oir_CreateReader,
		Oir_DeleteReader,
		Oir_SizeX,
		Oir_SizeY,
		Oir_SizeC,
		Oir_SizeZ,
		Oir_SizeT,
		Oir_SeriesInterval,
		Oir_DeviceColors,
		Oir_ReadPixels,
		Oir_ReadToPointer,

		// OmeTiffRWer
		Tiff_OpenRead,
		Tiff_OpenRW,
		Tiff_OpenCreate,
		Tiff_PixelType,
		Tiff_DimensionOrder,
		Tiff_SizeP,
		Tiff_SizeX,
		Tiff_SizeY,
		Tiff_SizeI,
		Tiff_SizeC,
		Tiff_SizeZ,
		Tiff_SizeT,
		Tiff_ChannelColors,
		Tiff_FileName,
		Tiff_ImageDescription,
		Tiff_ModifyParameters,
		Tiff_ReadPixels,
		Tiff_ReadPixelsI,
		Tiff_WritePixels,
		Tiff_WritePixelsI,
		Tiff_PixelPointer,
		Tiff_PixelPointerI,
		Tiff_ReadToPointer,
		Tiff_ReadToPointerI,
		Tiff_WriteFromPointer,
		Tiff_WriteFromPointerI,
		Tiff_Close,
	};
	try
	{
		API调用;
		const StructArray 成功结构 = 异常转结构(Image5D::成功异常);
		outputs[0] = 成功结构;
	}
	catch (const Image5D::Image5D异常& 异常)
	{
		outputs[0] = 异常转结构(异常);
		异常输出补全(outputs);
	}
}