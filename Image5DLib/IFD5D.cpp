#include "IFD5D.h"
using namespace Image5D;
using TagID = 字节序::TagID;
using TagType = 字节序::TagType;
IFD5D::IFD5D() :
	IFD(13),
	图像描述(Tag{ .Identifier = TagID::ImageDescription, .DataType = TagType::ASCII }),
	像素偏移(Tag{ .Identifier = TagID::StripOffsets, .DataType = TagType::LONG8, .NoValues = 1 }),
	图像宽度(Tag{ .Identifier = TagID::ImageWidth, .DataType = TagType::SHORT, .NoValues = 1 }),
	图像长度(Tag{ .Identifier = TagID::ImageLength, .DataType = TagType::SHORT, .NoValues = 1 }),
	每个样本的位数(Tag{ .Identifier = TagID::BitsPerSample, .DataType = TagType::SHORT, .NoValues = 1 }),
	每条行数(Tag{ .Identifier = TagID::RowsPerStrip, .DataType = TagType::SHORT, .NoValues = 1 }),
	像素字节数(Tag{ .Identifier = TagID::StripByteCounts, .DataType = TagType::LONG, .NoValues = 1 }),
	样本格式(Tag{ .Identifier = TagID::SampleFormat, .DataType = TagType::SHORT, .NoValues = 1 }),
	压缩(Tag{ .Identifier = TagID::Compression, .DataType = TagType::SHORT, .NoValues = 1,.Compression枚举 = 字节序::Compression::NoCompression }),
	光度解释(Tag{ .Identifier = TagID::PhotometricInterpretation, .DataType = TagType::SHORT, .NoValues = 1,.PhotometricInterpretation枚举 = 字节序::PhotometricInterpretation::BlackIsZero }),
	X分辨率(Tag{ .Identifier = TagID::XResolution, .DataType = TagType::RATIONAL, .NoValues = 1 }),
	Y分辨率(Tag{ .Identifier = TagID::YResolution, .DataType = TagType::RATIONAL, .NoValues = 1 }),
	分辨率单位(Tag{ .Identifier = TagID::ResolutionUnit, .DataType = TagType::SHORT, .NoValues = 1,.ResolutionUnit枚举 = 字节序::ResolutionUnit::NoUnit })
{
	X分辨率.扩展值.RATIONAL值 = 字节序::Rational{ .Numerator = 1,.Denominator = 1 };
	Y分辨率.扩展值.RATIONAL值 = 字节序::Rational{ .Numerator = 1,.Denominator = 1 };
}
IFD5D IFD5D::创建(uint64_t ImageDescription长度, 文件偏移<R_s<char>>ImageDescription偏移, 文件偏移<R_s<char>> StripOffsets, uint8_t SizeP, uint16_t SizeX, uint16_t SizeY, 字节序::SampleFormat SF)
{
	static IFD5D 模板 = IFD5D();
	IFD5D 新对象 = 模板;
	新对象.图像描述.NoValues = ImageDescription长度;
	新对象.图像描述.ASCII偏移 = ImageDescription偏移;
	新对象.像素偏移.ASCII偏移 = StripOffsets;
	新对象.图像宽度.SHORT值 = SizeX;
	新对象.图像长度.SHORT值 = SizeY;
	新对象.每个样本的位数.SHORT值 = SizeP * 8;
	新对象.每条行数.SHORT值 = SizeY;
	新对象.像素字节数.LONG值 = uint32_t(SizeP) * SizeX * SizeY;
	新对象.样本格式.SampleFormat枚举 = SF;
	return 新对象;
}