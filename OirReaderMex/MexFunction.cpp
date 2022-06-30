#include "pch.h"
#include "异常转换.h"
#include "Oir读入器.h"
API声明(CreateOirReader)
{
	const String 文件路径 = 万能转码<String>(inputs[1]);
#ifdef _DEBUG
	Oir读入器* 指针 = new Oir读入器((wchar_t*)文件路径.c_str());
	outputs[0] = 万能转码(指针);
#else
	outputs[0] = 万能转码(new Oir读入器((wchar_t*)文件路径.c_str()));
#endif
}
API声明(DestroyOirReader)
{
#ifdef _DEBUG
	Oir读入器* 指针 = 万能转码<Oir读入器*>(inputs[1]);
	delete 指针;
#else
	delete 万能转码<Oir读入器*>(inputs[1]);
#endif
	outputs[0] = 异常转换(成功);
}
API声明(SizeX)
{
	outputs[0] = 万能转码(万能转码<const Oir读入器*>(inputs[1])->SizeX());
}
API声明(SizeY)
{
	outputs[0] = 万能转码(万能转码<const Oir读入器*>(inputs[1])->SizeY());
}
API声明(SizeC)
{
	outputs[0] = 万能转码(万能转码<const Oir读入器*>(inputs[1])->SizeC());
}
API声明(SizeZ)
{
	outputs[0] = 万能转码(万能转码<const Oir读入器*>(inputs[1])->SizeZ());
}
API声明(SizeT)
{
	outputs[0] = 万能转码(万能转码<const Oir读入器*>(inputs[1])->SizeT());
}
API声明(SeriesInterval)
{
	outputs[0] = 万能转码(万能转码<const Oir读入器*>(inputs[1])->系列间隔());
}
API声明(DeviceColors)
{
	const Oir读入器* const 指针 = 万能转码<Oir读入器*>(inputs[1]);
	const uint8_t SizeC = 指针->SizeC();
	const uint8_t 颜色尺寸 = SizeC * 3;
	buffer_ptr_t<float> 颜色缓冲 = 数组工厂.createBuffer<float>(颜色尺寸);
	float* 颜色缓冲头 = 颜色缓冲.get();
	StringArray 设备缓冲 = 数组工厂.createArray<MATLABString>({ SizeC });
	TypedIterator<MATLABString> 设备缓冲头 = 设备缓冲.begin();
	const 设备颜色* 通道颜色 = 指针->通道颜色();
	for (const 设备颜色* const 通道尾 = 通道颜色 + SizeC; 通道颜色 < 通道尾; 通道颜色++)
	{
		*(设备缓冲头++) = 万能转码<MATLABString>(通道颜色->设备名称);
		*(颜色缓冲头++) = 通道颜色->红;
		*(颜色缓冲头++) = 通道颜色->绿;
		*(颜色缓冲头++) = 通道颜色->蓝;
	}
	outputs[0] = 设备缓冲;
	outputs[1] = 数组工厂.createArrayFromBuffer({ SizeC,3 }, std::move(颜色缓冲), MemoryLayout::ROW_MAJOR);
}
API声明(ReadPixels)
{
	const Oir读入器* const 指针 = 万能转码<Oir读入器*>(inputs[1]);
	const uint16_t SizeX = 指针->SizeX();
	const uint16_t SizeY = 指针->SizeY();
	const uint8_t SizeZ = 指针->SizeZ();
	switch (inputs.size())
	{
	case 4:
	{
		const uint16_t TSize = 万能转码<uint16_t>(inputs[3]);
		const uint8_t SizeC = 指针->SizeC();
		const uint64_t 尺寸 = uint64_t(TSize) * SizeZ * SizeC * SizeY * SizeX;
		buffer_ptr_t<uint16_t> 缓冲 = 数组工厂.createBuffer<uint16_t>(尺寸);
		outputs[0] = 万能转码(指针->读入像素(缓冲.get(), 万能转码<uint16_t>(inputs[2]), TSize));
		outputs[1] = 数组工厂.createArrayFromBuffer({ SizeX,SizeY,SizeC,SizeZ,TSize }, std::move(缓冲));
	}
	break;
	case 5:
	{
		const uint16_t TSize = 万能转码<uint16_t>(inputs[3]);
		const uint64_t 尺寸 = uint64_t(TSize) * SizeZ * SizeY * SizeX;
		buffer_ptr_t<uint16_t> 缓冲 = 数组工厂.createBuffer<uint16_t>(尺寸);
		outputs[0] = 万能转码(指针->读入像素(缓冲.get(), 万能转码<uint16_t>(inputs[2]), TSize, 万能转码<uint8_t>(inputs[4])));
		outputs[1] = 数组工厂.createArrayFromBuffer({ SizeX,SizeY,1,SizeZ,TSize }, std::move(缓冲));
	}
	break;
	case 8:
	{
		const uint16_t TSize = 万能转码<uint16_t>(inputs[3]);
		const uint8_t ZSize = 万能转码<uint8_t>(inputs[5]);
		const uint8_t CSize = 万能转码<uint8_t>(inputs[7]);
		const uint64_t 尺寸 = uint64_t(TSize) * ZSize * CSize * SizeY * SizeX;
		buffer_ptr_t<uint16_t> 缓冲 = 数组工厂.createBuffer<uint16_t>(尺寸);
		outputs[0] = 万能转码(指针->读入像素(缓冲.get(), 万能转码<uint16_t>(inputs[2]), TSize, 万能转码<uint8_t>(inputs[4]), ZSize, 万能转码<uint8_t>(inputs[6]), CSize));
		outputs[1] = 数组工厂.createArrayFromBuffer({ SizeX,SizeY,CSize,ZSize,TSize }, std::move(缓冲)); 
	}
		break;
	default:
		throw FeatureNotSupportedException("输入参数个数错误，不支持此重载");
	}
}
void MexFunction::operator()(ArgumentList outputs, ArgumentList inputs)
{
	API索引
	{
		CreateOirReader,
		DestroyOirReader,
		SizeX,
		SizeY,
		SizeC,
		SizeZ,
		SizeT,
		SeriesInterval,
		DeviceColors,
		ReadPixels,
	};
	try
	{
		API调用;
	}
	catch (Image5D异常 异常)
	{
		outputs[0] = 异常转换(异常);
	}
}