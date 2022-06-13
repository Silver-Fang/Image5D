#pragma once
#ifdef IMAGE5D_EXPORTS
#define Image5DAPI __declspec(dllexport)
#else
#define Image5DAPI __declspec(dllimport)
#endif
enum Image5D异常类型
{
	操作成功,
	文件打开失败,
	内存映射失败,
	文件不包含块,
	REF块不完整,
	XML块不完整,
	UID块不完整,
	像素块不完整,
	头文件不完整,
	图像属性不完整,
	图像属性解析失败,
	图像采集未定义,
	成像参数未定义,
	Z层尺寸未定义,
	Z层名称未定义,
	通道未定义,
	通道enable未定义,
	通道id未定义,
	通道order未定义,
	通道设备名未定义,
	扫描类型未定义,
	扫描设置未定义,
	扫描宽度未定义,
	扫描高度未定义,
	系列间隔未定义,
	找不到帧标头,
	找不到图像标头,
	找不到查找表,
	查找表解析失败,
	红色分量未定义,
	绿色分量未定义,
	蓝色分量未定义,
	空的像素块,
	未实现的方法,
	文件头不完整,
	不支持的版本号,
	头IFD不完整,
	图像描述不完整,
	图像描述解析失败,
	缺少OME节点,
	缺少UUID属性,
	缺少Image节点,
	缺少Name属性,
	缺少Pixels节点,
	缺少SizeX属性,
	缺少SizeY属性,
	缺少SizeC属性,
	缺少SizeZ属性,
	缺少SizeT属性,
	缺少DimensionOrder属性,
	维度顺序无效,
	缺少Type属性,
	SizeC与颜色数不匹配,
	缺少Color属性,
	像素类型无效,
	指定维度越界,
	SizeX为0,
	SizeY为0,
	SizeC为0,
	SizeZ为0,
	SizeT为0,
	构造参数无效,
	颜色指针为NULL,
	图像描述为NULL,
	修改参数无效,
	像素偏移超出文件范围,
	缺少必需标签,
	SizeCZT与SizeI不匹配,
	BitsPerSample与PixelType不匹配,
};
enum XML解析状态
{
	status_ok = 0,				// No error

	status_file_not_found,		// File was not found during load_file()
	status_io_error,			// Error reading from file/stream
	status_out_of_memory,		// Could not allocate memory
	status_internal_error,		// Internal error occurred

	status_unrecognized_tag,	// Parser could not determine tag type

	status_bad_pi,				// Parsing error occurred while parsing document declaration/processing instruction
	status_bad_comment,			// Parsing error occurred while parsing comment
	status_bad_cdata,			// Parsing error occurred while parsing CDATA section
	status_bad_doctype,			// Parsing error occurred while parsing document type declaration
	status_bad_pcdata,			// Parsing error occurred while parsing PCDATA section
	status_bad_start_element,	// Parsing error occurred while parsing start element tag
	status_bad_attribute,		// Parsing error occurred while parsing element attribute
	status_bad_end_element,		// Parsing error occurred while parsing end element tag
	status_end_element_mismatch,// There was a mismatch of start-end tags (closing tag had incorrect name, some tag was not closed or there was an excessive closing tag)

	status_append_invalid_root,	// Unable to append nodes since root type is not node_element or node_document (exclusive to xml_node::append_buffer)

	status_no_document_element	// Parsing resulted in a document without element nodes
};
struct Image5D异常
{
	//Win32错误代码放在最前面，以便第一时间获取
	union
	{
		DWORD Win32错误代码;
		XML解析状态 XML错误代码;
	};
	Image5D异常类型 类型;
};
constexpr Image5D异常 成功{ .类型 = 操作成功 };
#include <stdlib.h>
template <typename T>
class 透明向量
{
	bool 需要释放 = true;
public:
	UINT32 长度 = 0;
	UINT32 容量 = 0;
	T* 指针 = nullptr;
	void 加尾(T 新元素)
	{
		if (++长度 > 容量)
			指针 = (T*)realloc(指针, sizeof(T) * (容量 = 长度 * 2));
		指针[长度 - 1] = 新元素;
	}
	T* 释放所有权() 
	{ 
		需要释放 = false;
		return 指针; 
	}
	~透明向量()
	{
		if (需要释放)
			free(指针);
	}
};
Image5DAPI Image5D异常 打开文件(LPCSTR 文件路径, HANDLE& 文件句柄, HANDLE& 映射句柄, LPVOID& 映射指针, LPVOID& 尾指针);
Image5DAPI Image5D异常 打开文件(LPCWSTR 文件路径, HANDLE& 文件句柄, HANDLE& 映射句柄, LPVOID& 映射指针, LPVOID& 尾指针);
Image5DAPI LPVOID Get尾指针(HANDLE 文件句柄, LPVOID 映射指针);