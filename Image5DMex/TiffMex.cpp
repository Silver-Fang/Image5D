#include <IOmeTiff读写器.h>
#include<Mex工具.h>
using namespace Image5D;
using namespace Mex工具;
constexpr Image5D异常 指针越界(输入指针访问越界);
constexpr Image5D异常 元素太少(输入数组元素太少);
IOmeTiff读写器* 取指针(const Array& 指针标量)
{
	IOmeTiff读写器* const 指针 = 万能转码<IOmeTiff读写器*>(指针标量);
	if (对象存在(指针))
		return 指针;
	else
		throw Image5D异常(无效指针);
}
API声明(Tiff_OpenRead)
{
	const String 字符串 = 万能转码<String>(inputs[1]);
	IOmeTiff读写器* const 对象指针 = IOmeTiff读写器::只读打开((wchar_t*)字符串.c_str());
	outputs[1] = 万能转码(对象指针);
	自动析构(对象指针);
}
API声明(Tiff_OpenRW)
{
	const String 字符串 = 万能转码<String>(inputs[1]);
	IOmeTiff读写器* const 对象指针 = IOmeTiff读写器::读写打开((wchar_t*)字符串.c_str());
	outputs[1] = 万能转码(对象指针);
	自动析构(对象指针);
}
API声明(Tiff_OpenCreate)
{
	const String 字符串 = 万能转码<String>(inputs[1]);
	IOmeTiff读写器* 对象指针;
	switch (inputs.size())
	{
	case 3:
	{
		const std::string 图像描述 = 万能转码(std::move(inputs[2]));
		对象指针 = IOmeTiff读写器::覆盖创建((wchar_t*)字符串.c_str(), 图像描述.c_str());
	}
	break;
	case 9:
	{
		TypedArray<uint32_t>颜色数组(std::move(inputs[5]));
		const uint8_t SizeC = 颜色数组.getNumberOfElements();
		const buffer_ptr_t<uint32_t> 颜色缓冲 = 颜色数组.release();
		对象指针 = IOmeTiff读写器::覆盖创建((wchar_t*)字符串.c_str(), (像素类型)万能转码<uint8_t>(inputs[2]), 万能转码<uint16_t>(inputs[3]), 万能转码<uint16_t>(inputs[4]), SizeC, 万能转码<uint16_t>(inputs[6]), 万能转码<uint32_t>(inputs[7]), (颜色*)颜色缓冲.get(), (维度顺序)万能转码<uint8_t>(inputs[8]));
	}
	break;
	default:
		throw 参数异常;
	}
	outputs[1] = 万能转码(对象指针);
	自动析构(对象指针);
}
API声明(Tiff_PixelType)
{
	if (inputs.size() > 2)
		取指针(inputs[1])->PixelType((像素类型)万能转码<uint8_t>(inputs[2]));
	else
		outputs[1] = 万能转码((uint8_t)取指针(inputs[1])->PixelType());
}
API声明(Tiff_DimensionOrder)
{
	if (inputs.size() > 2)
		取指针(inputs[1])->DimensionOrder((维度顺序)万能转码<uint8_t>(inputs[2]));
	else
		outputs[1] = 万能转码((uint8_t)取指针(inputs[1])->DimensionOrder());
}
API声明(Tiff_SizeP)
{
	outputs[1] = 万能转码(取指针(inputs[1])->SizeP());
}
#define 尺寸属性(类型,字段) API声明(Tiff_##字段)\
{\
	if (inputs.size() > 2)\
		取指针(inputs[1])->字段(万能转码<类型>(inputs[2]));\
	else\
		outputs[1] = 万能转码(取指针(inputs[1])->字段());\
}
尺寸属性(UINT16, SizeX);
尺寸属性(UINT16, SizeY);
尺寸属性(UINT32, SizeI);
尺寸属性(uint8_t, SizeC);
尺寸属性(uint8_t, SizeZ);
尺寸属性(uint32_t, SizeT);
API声明(Tiff_ChannelColors)
{
	if (inputs.size() > 2)
	{
		IOmeTiff读写器* 对象指针 = 取指针(inputs[1]);
		TypedArray<uint32_t> 颜色数组(std::move(inputs[2]));
		if (对象指针->SizeC() > 颜色数组.getNumberOfElements())
			throw Image5D异常(颜色数与SizeC不匹配);
		const buffer_ptr_t<uint32_t> 颜色缓冲 = 颜色数组.release();
		对象指针->通道颜色((颜色*)颜色缓冲.get());
	}
	else
	{
		const IOmeTiff读写器* 对象指针 = 取指针(inputs[1]);
		const uint8_t SizeC = 对象指针->SizeC();		
		outputs[1] = 万能转码((UINT32*)对象指针->通道颜色(), { SizeC,1 });
	}
}
API声明(Tiff_FileName)
{
	if (inputs.size() > 2)
	{
		const std::string 字符串 = 万能转码(std::move(inputs[2]));
		取指针(inputs[1])->文件名(字符串.c_str());
	}
	else
		outputs[1] = 万能转码<CharArray>(取指针(inputs[1])->文件名());
}
API声明(Tiff_ImageDescription)
{
	if (inputs.size() > 2)
		取指针(inputs[1])->图像描述(万能转码(std::move(inputs[2])));
	else
		outputs[1] = 万能转码<CharArray>(取指针(inputs[1])->图像描述());
}
API声明(Tiff_ModifyParameters)
{
	IOmeTiff读写器* 对象指针 = 取指针(inputs[1]);
	TypedArray<uint32_t>颜色数组(std::move(inputs[8]));
	const bool 指定值 = 颜色数组.getNumberOfElements();
	const buffer_ptr_t<uint32_t>颜色缓冲 = 颜色数组.release();
	const 颜色* const 颜色指针 = 指定值 ? (颜色*)颜色缓冲.get() : nullptr;
	const std::string 文件名 = 万能转码(std::move(inputs[10]));
	const char* 文件名指针 = 文件名.c_str();
	if (!*文件名指针)
		文件名指针 = nullptr;
	对象指针->修改参数((像素类型)万能转码<uint8_t>(inputs[2]), 万能转码<UINT16>(inputs[3]), 万能转码<UINT16>(inputs[4]), 万能转码<uint8_t>(inputs[5]), 万能转码<uint8_t>(inputs[6]), 万能转码<uint32_t>(inputs[7]), 颜色指针, (维度顺序)万能转码<uint8_t>(inputs[9]), 文件名指针);
}
ArrayDimensions CZT重排(维度顺序 DO, uint16_t X, uint16_t Y, uint8_t C, uint8_t Z, uint32_t T)
{
	switch (DO)
	{
	case XYCZT:
		return { X,Y,C,Z,T };
	case XYCTZ:
		return { X,Y,C,T,Z };
	case XYZCT:
		return { X,Y,Z,C,T };
	case XYZTC:
		return { X,Y,Z,T,C };
	case XYTCZ:
		return { X,Y,T,C,Z };
	case XYTZC:
		return { X,Y,T,Z,C };
	}
}
constexpr ArrayType 类型映射[] = { ArrayType::UINT8,ArrayType::UINT16,ArrayType::UINT32,ArrayType::INT8,ArrayType::INT16,ArrayType::INT32,ArrayType::SINGLE,ArrayType::DOUBLE };
API声明(Tiff_ReadPixels)
{
	const IOmeTiff读写器* const 对象指针 = 取指针(inputs[1]);
	const ArrayType 类型 = 类型映射[(size_t)对象指针->PixelType()];
	const uint64_t 图面像素数 = (uint64_t)对象指针->SizeX() * 对象指针->SizeY();
	switch (inputs.size())
	{
	case 2:
	{
		const std::unique_ptr<动态类型缓冲>缓冲 = 动态类型缓冲::创建(类型, 图面像素数 * 对象指针->SizeI());
		对象指针->读入像素(缓冲->指针);
		outputs[1] = 缓冲->打包(CZT重排(对象指针->DimensionOrder(), 对象指针->SizeX(), 对象指针->SizeY(), 对象指针->SizeC(), 对象指针->SizeZ(), 对象指针->SizeT()));
	}
	break;
	case 4:
	{
		const uint32_t TSize = 万能转码<uint32_t>(inputs[3]);
		const std::unique_ptr<动态类型缓冲>缓冲 = 动态类型缓冲::创建(类型, 图面像素数 * TSize * 对象指针->SizeZ() * 对象指针->SizeC());
		对象指针->读入像素(缓冲->指针, 万能转码<uint32_t>(inputs[2]), TSize);
		outputs[1] = 缓冲->打包(CZT重排(对象指针->DimensionOrder(), 对象指针->SizeX(), 对象指针->SizeY(), 对象指针->SizeC(), 对象指针->SizeZ(), TSize));
	}
	break;
	case 8:
	{
		const uint32_t TSize = 万能转码<uint32_t>(inputs[3]);
		const uint8_t ZSize = 万能转码<uint8_t>(inputs[5]);
		const uint8_t CSize = 万能转码<uint8_t>(inputs[7]);
		const std::unique_ptr<动态类型缓冲>缓冲 = 动态类型缓冲::创建(类型, 图面像素数 * TSize * ZSize * CSize );
		对象指针->读入像素(缓冲->指针, 万能转码<uint32_t>(inputs[2]), TSize, 万能转码<uint8_t>(inputs[4]), ZSize, 万能转码<uint8_t>(inputs[6]), CSize);
		outputs[1] = 缓冲->打包(CZT重排(对象指针->DimensionOrder(), 对象指针->SizeX(), 对象指针->SizeY(), CSize, ZSize, TSize));
	}
	break;
	default:
		throw 参数异常;
	}
}
API声明(Tiff_ReadPixelsI)
{
	const IOmeTiff读写器* const 对象指针 = 取指针(inputs[1]);
	const UINT32 ISize = 万能转码<UINT32>(inputs[3]);
	const std::unique_ptr<动态类型缓冲>缓冲 = 动态类型缓冲::创建(类型映射[(size_t)对象指针->PixelType()], (uint64_t)ISize * 对象指针->SizeX() * 对象指针->SizeY());
	对象指针->读入像素(缓冲->指针, 万能转码<UINT32>(inputs[2]), ISize);
	outputs[1] = 缓冲->打包({ 对象指针->SizeX(),对象指针->SizeY(),ISize });
}
API声明(Tiff_WritePixels)
{
	const IOmeTiff读写器* const 对象指针 = 取指针(inputs[1]);
	const std::unique_ptr<动态类型缓冲>缓冲 = 动态类型缓冲::读取(std::move(inputs[2]));
	const uint64_t 图面字节数 = (uint64_t)对象指针->SizeX() * 对象指针->SizeY() * 对象指针->SizeP();
	switch (inputs.size())
	{
	case 3:
	{
		if (缓冲->字节数 < 图面字节数 * 对象指针->SizeT() * 对象指针->SizeZ() * 对象指针->SizeC())
			throw 元素太少;
		对象指针->写出像素(缓冲->指针);
	}
	break;
	case 5:
	{
		const uint32_t TSize = 万能转码<uint32_t>(inputs[4]);
		if (缓冲->字节数 < 图面字节数 * TSize * 对象指针->SizeZ() * 对象指针->SizeC())
			throw 元素太少;
		对象指针->写出像素(缓冲->指针, 万能转码<uint32_t>(inputs[3]), TSize);
	}
	break;
	case 9:
	{
		const uint32_t TSize = 万能转码<uint32_t>(inputs[4]);
		const uint8_t ZSize = 万能转码<uint8_t>(inputs[6]);
		const uint8_t CSize = 万能转码<uint8_t>(inputs[8]);
		if (缓冲->字节数 < 图面字节数 * TSize * ZSize * CSize)
			throw 元素太少;
		对象指针->写出像素(缓冲->指针, 万能转码<uint32_t>(inputs[3]), TSize, 万能转码<uint8_t>(inputs[5]), ZSize, 万能转码<uint8_t>(inputs[7]), CSize);
	}
	break;
	default:
		throw 参数异常;
	}
}
API声明(Tiff_WritePixelsI)
{
	const IOmeTiff读写器* const 对象指针 = 取指针(inputs[1]);
	const std::unique_ptr<动态类型缓冲>缓冲 = 动态类型缓冲::读取(std::move(inputs[2]));
	const uint32_t ISize = 万能转码<UINT32>(inputs[4]);
	if (缓冲->字节数 < (size_t)ISize * 对象指针->SizeX() * 对象指针->SizeY() * 对象指针->SizeP())
		throw 元素太少;
	对象指针->写出像素I(缓冲->指针, 万能转码<UINT32>(inputs[3]), ISize);
}
API声明(Tiff_PixelPointer)
{
	const IOmeTiff读写器* 对象指针 = 取指针(inputs[1]);
	void* 像素指针;
	size_t 容量;
	switch (inputs.size())
	{
	case 2:
		对象指针->像素指针(像素指针, 容量);
		break;
	case 3:
		对象指针->像素指针(万能转码<uint32_t>(inputs[2]), 像素指针, 容量);
		break;
	case 5:
		对象指针->像素指针(万能转码<uint32_t>(inputs[2]), 万能转码<uint8_t>(inputs[3]), 万能转码<uint8_t>(inputs[4]), 像素指针, 容量);
		break;
	default:
		throw 参数异常;
	}
	outputs[1] = 万能转码(像素指针);
	outputs[2] = 万能转码(容量);
}
API声明(Tiff_PixelPointerI)
{
	const IOmeTiff读写器* 对象指针 = 取指针(inputs[1]);
	void* 像素指针;
	size_t 容量;
	对象指针->像素指针I(万能转码<UINT32>(inputs[2]), 像素指针, 容量);
	outputs[1] = 万能转码(像素指针);
	outputs[2] = 万能转码(容量);
}
API声明(Tiff_ReadToPointer)
{
	const IOmeTiff读写器* const 对象指针 = 取指针(inputs[1]);
	void* const 输出指针 = 万能转码<void*>(inputs[2]);
	const size_t 输出容量 = 万能转码<size_t>(inputs[3]);
	const size_t SizePXY = 对象指针->SizeP() * 对象指针->SizeX() * 对象指针->SizeY();
	switch (inputs.size())
	{
	case 4:
		if (SizePXY * 对象指针->SizeT() * 对象指针->SizeZ() * 对象指针->SizeC() > 输出容量)
			throw 内存溢出;
		对象指针->读入像素(输出指针);
		break;
	case 6:
	{
		const uint32_t TSize = 万能转码<uint32_t>(inputs[5]);
		if (SizePXY * TSize * 对象指针->SizeZ() * 对象指针->SizeC() > 输出容量)
			throw 内存溢出;
		对象指针->读入像素(输出指针, 万能转码<uint32_t>(inputs[4]), TSize);
	}
	break;
	case 10:
	{
		const uint32_t TSize = 万能转码<uint32_t>(inputs[5]);
		const uint8_t ZSize = 万能转码<uint8_t>(inputs[7]);
		const uint8_t CSize = 万能转码<uint8_t>(inputs[9]);
		if (SizePXY * TSize * ZSize * CSize > 输出容量)
			throw 内存溢出;
		对象指针->读入像素(输出指针, 万能转码<uint32_t>(inputs[4]), TSize, 万能转码<uint8_t>(inputs[6]), ZSize, 万能转码<uint8_t>(inputs[8]), CSize);
	}
	break;
	default:
		throw 参数异常;
	}
}
API声明(Tiff_ReadToPointerI)
{
	const IOmeTiff读写器* const 对象指针 = 取指针(inputs[1]);
	void* const 输出指针 = 万能转码<void*>(inputs[2]);
	const size_t 输出容量 = 万能转码<size_t>(inputs[3]);
	const size_t SizePXY = 对象指针->SizeP() * 对象指针->SizeX() * 对象指针->SizeY();
	const uint32_t ISize = 万能转码<uint32_t>(inputs[5]);
	if (SizePXY * ISize > 输出容量)
		throw 内存溢出;
	对象指针->读入像素I(输出指针, 万能转码<uint32_t>(inputs[4]), ISize);
}
API声明(Tiff_WriteFromPointer)
{
	const IOmeTiff读写器* const 对象指针 = 取指针(inputs[1]);
	const void* const 输入指针 = 万能转码<void*>(inputs[2]);
	const size_t 输入容量 = 万能转码<size_t>(inputs[3]);
	const size_t SizePXY = 对象指针->SizeP() * 对象指针->SizeX() * 对象指针->SizeY();
	switch (inputs.size())
	{
	case 4:
		if (SizePXY * 对象指针->SizeT() * 对象指针->SizeZ() * 对象指针->SizeC() > 输入容量)
			throw 指针越界;
		对象指针->写出像素(输入指针);
		break;
	case 6:
	{
		const uint32_t TSize = 万能转码<uint32_t>(inputs[5]);
		if (SizePXY * TSize * 对象指针->SizeZ() * 对象指针->SizeC() > 输入容量)
			throw 指针越界;
		对象指针->写出像素(输入指针, 万能转码<uint32_t>(inputs[4]), TSize);
	}
	break;
	case 10:
	{
		const uint32_t TSize = 万能转码<uint32_t>(inputs[5]);
		const uint8_t ZSize = 万能转码<uint8_t>(inputs[7]);
		const uint8_t CSize = 万能转码<uint8_t>(inputs[9]);
		if (SizePXY * TSize * ZSize * CSize > 输入容量)
			throw 指针越界;
		对象指针->写出像素(输入指针, 万能转码<uint32_t>(inputs[4]), TSize, 万能转码<uint8_t>(inputs[6]), ZSize, 万能转码<uint8_t>(inputs[8]), CSize);
	}
	break;
	default:
		throw 参数异常;
	}
}
API声明(Tiff_WriteFromPointerI)
{
	const IOmeTiff读写器* const 对象指针 = 取指针(inputs[1]);
	const void* const 输入指针 = 万能转码<void*>(inputs[2]);
	const size_t 输入容量 = 万能转码<size_t>(inputs[3]);
	const size_t SizePXY = 对象指针->SizeP() * 对象指针->SizeX() * 对象指针->SizeY();
	const uint32_t ISize = 万能转码<uint32_t>(inputs[5]);
	if (SizePXY * ISize > 输入容量)
		throw 指针越界;
	对象指针->写出像素I(输入指针, 万能转码<uint32_t>(inputs[4]), ISize);
}
API声明(Tiff_Close)
{
	IOmeTiff读写器* const 对象指针 = 万能转码<IOmeTiff读写器*>(inputs[1]);
	if (手动析构(对象指针))
		delete 对象指针;
}