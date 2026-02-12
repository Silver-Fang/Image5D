#include "Oir读入器.h"
import std;
using namespace Image5D;
enum class Oir基块类型 :uint32_t
{
	元数据,
	帧属性,
	BMP,
	UID,
	像素,
	空,
};
enum class 元数据类型 :uint32_t
{
	fileinfo = 1,
	lsmimage,
	annotation,
	overlay,
	lut,
};
#pragma pack(push,4)
struct Oir文件头
{
	char OLYMPUSFORMAT[16] = { 'O','L','Y','M','P','U','S','R','A','W','F','O','R','M','A','T' };
	uint32_t 未知字段[4] = { 12,0,1,2 };
	uint64_t 文件大小;
	uint64_t 索引位置;
};
struct Oir基块
{
	uint32_t 长度;
	Oir基块类型 类型;
};
struct UID块 :Oir基块
{
	uint32_t 前像素块长度;
	uint32_t 后像素块长度;
	uint32_t UID长度;
};
struct 元数据块
{
	元数据类型 类型;
	uint32_t 未知字段[8];
	uint32_t 长度;
};
#pragma pack(pop)
using namespace pugi;
template<typename T>
struct Z设置
{
	float 深度;
	T 设置值;
};
template<typename T>
void 深度参数生成(const std::vector<Z设置<T>>&设置向量, float 深度, uint8_t SizeZ, T* 输出头,float Z步长)
{
	T* const 输出尾 = 输出头 + SizeZ;
	typename std::vector<Z设置<T>>::const_iterator 下一个 = 设置向量.cbegin();
	const int8_t 前余Z = (std::min)(uint8_t((下一个->深度 - 深度) / Z步长 + 1), SizeZ);
	if (前余Z > 0)
		输出头 = std::fill_n(输出头, 前余Z, 下一个->设置值);
	深度 += 前余Z * Z步长;
	bool 区间切换;
	float 设置深度比;
	typename std::vector<Z设置<T>>::const_iterator 上一个;
	while (输出头 < 输出尾)
	{
		while (深度 > 下一个->深度)
		{
			if (++下一个 == 设置向量.cend())
			{
				std::fill(输出头, 输出尾, (下一个 - 1)->设置值);
				return;
			}
			区间切换 = true;
		}
		if (区间切换)
		{
			上一个 = 下一个 - 1;
			设置深度比 = (下一个->设置值 - 上一个->设置值) / (下一个->深度 - 上一个->深度);
			区间切换 = false;
		}
		*(输出头++) = 设置深度比 * (深度 - 上一个->深度) + 上一个->设置值;
		深度 += Z步长;
	}
}
void Oir读入器::创建新索引(const wchar_t* 字符缓冲)
{
	索引文件.reset();
	try
	{
		索引文件 = std::make_unique<文件映射>(字符缓冲, 1ll); //这里必须1ll，否则重载不明确
	}
	catch(Image5D::Win32异常& e)
	{
		e.Image5D异常 = Image5D::Exception::Unable_to_create_index_file;
		throw e;
	}
	文件列表类::const_iterator 当前文件 = 文件列表.cbegin();
	const Oir文件头* 文件头 = (Oir文件头*)(*当前文件)->映射指针();
	const char* const 全局文件头 = (char*)文件头;
	const void* 尾指针 = (char*)文件头 + (*当前文件)->文件大小();
	if (文件头 + 1 > 尾指针)
		throw Image5D::Exception::Oir_file_header_is_incomplete;
	const uint64_t* 基块索引 = (uint64_t*)((char*)文件头 + 文件头->索引位置 + 4);
	const Oir基块* 基块指针;
	do
	{
		if (基块索引 + 1 > 尾指针)
			throw Image5D::Exception::Oir_base_block_index_error;
		if ((基块指针 = (Oir基块*)((char*)文件头 + *(基块索引++))) + 1 > 尾指针)
			throw Image5D::Exception::Oir_base_block_index_error;
	} while (基块指针->类型 != Oir基块类型::帧属性);//此循环用于跳过REF块
	if (基块索引 + 1 > 尾指针)
		throw Image5D::Exception::Oir_base_block_index_error;
	const UID块* UID块指针 = (UID块*)((char*)文件头 + *(基块索引++));
	const char* 字符串 = (char*)(UID块指针 + 1);
	if (字符串 > 尾指针)
		throw Image5D::Exception::UID_block_is_incomplete;
	std::vector<uint32_t> 每块像素数向量;
	const bool 有Z = *字符串 == 'z';
	const uint16_t* 像素指针;
	do
	{
		if (基块索引 + 1 > 尾指针)
			throw Image5D::Exception::Oir_base_block_index_error;
		if ((像素指针 = (uint16_t*)((基块指针 = (Oir基块*)((char*)文件头 + *(基块索引++))) + 1)) > 尾指针)
			throw Image5D::Exception::Oir_base_block_index_error;
		每块像素数向量.push_back(基块指针->长度 / 2);
		if ((char*)像素指针 + 基块指针->长度 > 尾指针)
			throw Image5D::Exception::The_pixel_block_is_incomplete;
		块指针.push_back(像素指针);
		if ((基块指针 = (Oir基块*)((char*)文件头 + *(基块索引++))) + 1 > 尾指针)
			throw Image5D::Exception::Oir_base_block_index_error;
	} while (基块指针->类型 == Oir基块类型::UID);
	//这里的每块像素数可能有C通道上的重复，没关系，后面得到SizeC以后会消掉
	const 元数据块* 元数据块指针 = (元数据块*)(基块指针 + 1);
	if ((字符串 = (char*)(元数据块指针 + 1)) > 尾指针)
		throw Image5D::Exception::The_Oir_metadata_block_is_incomplete;
	if ((字符串 = (char*)((元数据块指针 = (元数据块*)(字符串 + 元数据块指针->长度)) + 1)) > 尾指针)//跳过fileinfo
		throw Image5D::Exception::The_Oir_metadata_block_is_incomplete;
	Oir索引 新索引;
	新索引.LsmimageXml长度 = 元数据块指针->长度;
	if ((元数据块指针 = (元数据块*)(字符串 + 新索引.LsmimageXml长度)) > 尾指针)
		throw Image5D::Exception::Oir_image_attribute_out_of_bounds;
	xml_document 图像属性文档;
	xml_parse_result 解析结果 = 图像属性文档.load_buffer(字符串, 新索引.LsmimageXml长度);
	if (解析结果.status != xml_parse_status::status_ok)
		throw 解析结果;
	新索引.LsmimageXml偏移 = 字符串 - 全局文件头;
	const xml_node 图像属性节点 = 图像属性文档.child("lsmimage:imageProperties");
	if (!图像属性节点)
		throw Image5D::Exception::The_image_property_is_not_defined;
	xml_node 节点 = 图像属性节点.child("commonimage:general");
	xml_text 节点文本;
	if (节点 && (节点 = 节点.child("base:creationDateTime")) && (节点文本 = 节点.text()))
		strcpy_s(新索引.创建日期时间, 节点文本.get());
	else
		throw Image5D::Exception::CreationDateTime_not_defined;
	const xml_node 采集节点 = 图像属性节点.child("commonimage:acquisition");
	if (!采集节点)
		throw Image5D::Exception::Image_acquisition_undefined;
	const xml_node 成像参数节点 = 采集节点.child("commonimage:imagingParam");
	if (!成像参数节点)
		throw Image5D::Exception::Imaging_parameters_are_not_defined;
	xml_attribute 节点属性;
	float Z起点;
	if (有Z)
	{
		新索引.SizeZ = 0;
		for (const xml_node 轴节点 : 成像参数节点.children("commonparam:axis"))
			if ((节点属性 = 轴节点.attribute("paramEnable")) && 节点属性.as_bool())
			{
				if ((节点 = 轴节点.child("commonparam:startPosition")) && (节点文本 = 节点.text()))
					Z起点 = 节点文本.as_float();
				else
					throw Image5D::Exception::The_Z_level_starting_point_is_not_defined;
				if ((节点 = 轴节点.child("commonparam:step")) && (节点文本 = 节点.text()))
					新索引.Z步长 = 节点文本.as_float();
				else
					throw Image5D::Exception::Z_layer_step_size_is_not_defined;
				if ((节点 = 轴节点.child("commonparam:maxSize")) && (节点文本 = 节点.text()))
					新索引.SizeZ = 节点文本.as_uint();
				else
					throw Image5D::Exception::Z_layer_size_is_not_defined;
				break;
			}
		if (!新索引.SizeZ)
			throw Image5D::Exception::The_z_axis_is_undefined;
	}
	else
	{
		新索引.SizeZ = 1;
		新索引.Z步长 = 0;
	}
	if (!(节点 = 采集节点.child("commonimage:phase")))
		throw Image5D::Exception::Image_phase_undefined;
	struct 通道设备
	{
		const char* 通道ID;
		uint8_t 顺序;
		const char* 探测器ID;
		const char* 设备名称;
	};
	std::vector<通道设备> 通道设备向量;
	uint8_t 通道组个数 = 0;
	for (xml_node 节点 : 节点.children("commonphase:group"))
	{
		for (xml_node const 通道 : 节点.children("commonphase:channel"))
		{
			通道组个数++;
			if (!(节点属性 = 通道.attribute("enable")))
				throw Image5D::Exception::Channels_enable_undefined;
			if (!节点属性.as_bool())
				continue;
			通道设备向量.emplace_back();
			通道设备& 通道设备对象 = 通道设备向量.back();
			if (节点属性 = 通道.attribute("id"))
				通道设备对象.通道ID = 节点属性.value();
			else
				throw Image5D::Exception::Channel_ID_not_defined;
			if (节点属性 = 通道.attribute("order"))
				通道设备对象.顺序 = 节点属性.as_uint();
			else
				throw Image5D::Exception::The_channel_order_is_undefined;
			if (节点属性 = 通道.attribute("detectorId"))
				通道设备对象.探测器ID = 节点属性.value();
			else
				throw Image5D::Exception::Detector_ID_not_defined;
			if ((节点 = 通道.child("commonphase:deviceName")) && (节点文本 = 节点.text()))
				通道设备对象.设备名称 = 节点文本.get();
			else
				throw Image5D::Exception::Channel_device_name_not_defined;
		}
	}
	if (通道设备向量.empty())
		throw Image5D::Exception::The_channel_is_not_defined;
	新索引.SizeC = 通道设备向量.size();
	std::sort(通道设备向量.begin(), 通道设备向量.end(), [](const 通道设备& 对象1, const 通道设备& 对象2) {return 对象1.顺序 <= 对象2.顺序; });
	std::unique_ptr<设备颜色[]> 通道颜色 = std::make_unique_for_overwrite<设备颜色[]>(新索引.SizeC);
	for (uint8_t C = 0; C < 新索引.SizeC; ++C)
		if (strcpy_s(通道颜色[C].设备名称, 通道设备向量[C].设备名称))
			throw Image5D::Exception::The_device_name_is_too_long;
	if ((节点 = 采集节点.child("commonimage:configuration")) && (节点 = 节点.child("lsmimage:scannerType")) && (节点文本 = 节点.text()))
		字符串 = 节点文本.as_string();
	else
		throw Image5D::Exception::Scan_type_not_defined;
	新索引.SizeX = 0;
	新索引.SizeY = 0;
	新索引.系列间隔 = 0;
	for (xml_node 节点 : 采集节点.children("lsmimage:scannerSettings"))
	{
		if (!(节点属性 = 节点.attribute("type")))
			throw Image5D::Exception::Scan_type_not_defined;
		if (strcmp(节点属性.value(), 字符串))
			continue;
		const xml_node 扫描参数 = 节点.child("lsmimage:param");
		if (!扫描参数)
			throw Image5D::Exception::Scan_parameters_are_not_defined;
		const xml_node 图像尺寸 = 扫描参数.child("lsmparam:imageSize");
		if (!图像尺寸)
			throw Image5D::Exception::Image_size_undefined;
		if ((节点 = 图像尺寸.child("commonparam:width")) && (节点文本 = 节点.text()))
			新索引.SizeX = 节点文本.as_uint();
		else
			throw Image5D::Exception::Scan_width_not_defined;
		if ((节点 = 图像尺寸.child("commonparam:height")) && (节点文本 = 节点.text()))
			新索引.SizeY = 节点文本.as_uint();
		else
			throw Image5D::Exception::Scan_height_is_not_defined;
		if ((节点 = 扫描参数.child("lsmparam:speed")) && (节点 = 节点.child("commonparam:speedInformation")) && (节点 = 节点.child("commonparam:seriesInterval")) && (节点文本 = 节点.text()))
			新索引.系列间隔 = 节点文本.as_float();
		else
			throw Image5D::Exception::Scan_speed_is_not_defined;
		break;
	}
	if (!(新索引.SizeX && 新索引.SizeY && 新索引.系列间隔))
		throw Image5D::Exception::Scan_Settings_not_defined;
	//不能复用图像解析文档，会损坏通道设备指针的通道
	xml_document LUT文档;
	if ((字符串 = (char*)(元数据块指针 + 1)) > 尾指针)
		throw Image5D::Exception::The_Oir_metadata_block_is_incomplete;
	if ((字符串 = (char*)((元数据块指针 = (元数据块*)(字符串 + 元数据块指针->长度)) + 1)) > 尾指针)//跳过annotation
		throw Image5D::Exception::The_Oir_metadata_block_is_incomplete;
	元数据块指针 = (元数据块*)(字符串 + 元数据块指针->长度);//跳过overlay
	const uint32_t* 长度指针 = (uint32_t*)(元数据块指针 + 1);
	for (uint8_t C1 = 0; C1 < 通道组个数; ++C1)
	{
		const char* const UID头 = (char*)(长度指针 + 1);
		if (UID头 > 尾指针)
			throw Image5D::Exception::LutUid_length_out_of_bounds;
		const char* const UID尾 = UID头 + *长度指针;
		if ((长度指针 = (uint32_t*)UID尾) > 尾指针)
			throw Image5D::Exception::LutUid_out_of_bounds;
		if ((字符串 = (char*)(长度指针 + 1)) > 尾指针)
			throw Image5D::Exception::LutXml_length_out_of_bounds;
		const uint32_t 长度 = *长度指针;
		if ((长度指针 = (uint32_t*)(字符串 + 长度)) > 尾指针)
			throw Image5D::Exception::LutXml_out_of_bounds;
		//确保即使没有找到也能正确跳到下一个LutXml指针位置
		for (uint8_t C2 = 0; C2 < 新索引.SizeC; ++C2)
			if (std::equal(UID头, UID尾, 通道设备向量[C2].通道ID))
			{
				if ((解析结果 = LUT文档.load_buffer(字符串, 长度)).status != xml_parse_status::status_ok)
					throw 解析结果;
				const xml_node LUT节点 = LUT文档.child("lut:LUT");
				if (!LUT节点)
					throw Image5D::Exception::Lookup_table_undefined;
				设备颜色& 通道 = 通道颜色[C2];
				if ((节点 = LUT节点.child("lut:red")) && (节点 = 节点.child("lut:contrast")) && (节点文本 = 节点.text()))
					通道.红 = 节点文本.as_float();
				else
					throw Image5D::Exception::The_red_component_is_undefined;
				if ((节点 = LUT节点.child("lut:green")) && (节点 = 节点.child("lut:contrast")) && (节点文本 = 节点.text()))
					通道.绿 = 节点文本.as_float();
				else
					throw Image5D::Exception::The_green_component_is_undefined;
				if ((节点 = LUT节点.child("lut:blue")) && (节点 = 节点.child("lut:contrast")) && (节点文本 = 节点.text()))
					通道.蓝 = 节点文本.as_float();
				else
					throw Image5D::Exception::The_blue_component_is_undefined;
				if ((节点 = LUT节点.child("lut:alpha")) && (节点 = 节点.child("lut:contrast")) && (节点文本 = 节点.text()))
					通道.不透明度 = 1 - 节点文本.as_float();
				else
					throw Image5D::Exception::The_opacity_is_undefined;
				break;
			}
	}
	const uint8_t 每层像素块数 = 每块像素数向量.size();
	const uint8_t 每层基块数 = (每层像素块数 + 1) * 2;
	基块索引 += 1;
	const uint8_t 每周期像素块数 = 每层像素块数 * 新索引.SizeZ;
	while (基块索引 + 每层基块数 < 尾指针)
	{
		for (uint8_t a = 0; a < 每层像素块数; ++a)
			块指针.push_back((uint16_t*)((Oir基块*)((char*)文件头 + *(基块索引 += 2)) + 1));
		基块索引 += 2;
	}
	const 文件列表类::const_iterator 文件结束 = 文件列表.cend();
	std::vector<uint32_t> 拼接SizeT;
	while (++当前文件 < 文件结束)
	{
		文件头 = (Oir文件头*)(*当前文件)->映射指针();
		尾指针 = (char*)文件头 + (*当前文件)->文件大小();
		if (reinterpret_cast<const char*>(尾指针) - reinterpret_cast<const char*>(文件头) < 文件头->索引位置) [[unlikely]]
		{
			//文件头损坏，放弃该文件。但是，需要在拼接SizeT中反映这一异常情况。
			拼接SizeT.push_back(块指针.size() / 每周期像素块数);

			//删除上个文件尾可能存在的多余的不完整周期
			块指针.resize(拼接SizeT.back() * 每周期像素块数);

			continue;
		}
		基块索引 = (uint64_t*)((char*)文件头 + 文件头->索引位置 + 4);
		do
		{
			if (基块索引 + 1 > 尾指针)
				[[unlikely]] goto 绝对跳出;
		} while (((Oir基块*)((char*)文件头 + *(基块索引++)))->类型 != Oir基块类型::帧属性);//跳过REF块
		if ((基块索引--) + 每层基块数 > 尾指针)
		{
			//文件头损坏，放弃该文件。但是，需要在拼接SizeT中反映这一异常情况。
			拼接SizeT.push_back(块指针.size() / 每周期像素块数);

			//删除上个文件尾可能存在的多余的不完整周期
			块指针.resize(拼接SizeT.back() * 每周期像素块数);

			continue;
		}
		//对于拼接的OIR序列，需要检测中间突然遇到的头文件，删除上个文件尾可能存在的多余的不完整周期。头文件的特征是，倒数第二块是BMP。
		if (((Oir基块*)((char*)文件头 + *((uint64_t*)尾指针 - 2)))->类型 == Oir基块类型::BMP) [[unlikely]]
		{
			//记录每个拼接的SizeT位置
			拼接SizeT.push_back(块指针.size() / 每周期像素块数);

			//删除上个文件尾可能存在的多余的不完整周期
			块指针.resize(拼接SizeT.back()* 每周期像素块数);
		}
		for (uint8_t a = 0; a < 每层像素块数; ++a)
			块指针.push_back((uint16_t*)((Oir基块*)((char*)文件头 + *(基块索引 += 2)) + 1));
		基块索引 += 3;
		while (基块索引 + 每层基块数 < 尾指针)
		{
			for (uint8_t a = 0; a < 每层像素块数; ++a)
				块指针.push_back((uint16_t*)((Oir基块*)((char*)文件头 + *(基块索引 += 2)) + 1));
			基块索引 += 2;
		}
	绝对跳出:;
	}
	//如果未发生拼接，拼接SizeT将为空，始终比实际拼接序列数少一个。但如果最后一个拼接值恰等于SizeT，说明拼接后没有任何数据，应当作最后一个拼接不存在处理。
	新索引.文件序列拼接数 = 拼接SizeT.empty() ? 1 : 拼接SizeT.size() + (拼接SizeT.back() < 块指针.size() / 每周期像素块数);
	新索引.每帧分块数 = 每层像素块数 / 新索引.SizeC;
	新索引.计算依赖字段(块指针.size());
	const uint32_t 文件大小 = 新索引.计算文件大小();
	索引文件->文件大小(文件大小);
	索引文件->映射指针(nullptr);
	*(索引 = (Oir索引*)索引文件->映射指针()) = 新索引;
	块指针.resize(索引->SizeTZBC);
	uint64_t* 块偏移;
	索引->Get变长成员(i激光透过率, i通道颜色, i放大电压, 每块像素数, 块偏移, _拼接SizeT);
	uint8_t 序列 = 索引->文件序列拼接数 - 1;
	if (序列)
	{
		//序列不为0意味着发生了拼接。最后一个序列的SizeT不能用拼接SizeT计算出
		_拼接SizeT[序列] = 索引->SizeT - 拼接SizeT.back();
		while (--序列)
			_拼接SizeT[序列] = 拼接SizeT[序列] - 拼接SizeT[序列 - 1];
		//第一个序列的SizeT不需要减
		_拼接SizeT[0] = 拼接SizeT[0];
	}
	else[[likely]]//序列为0表示未拼接，直接等于SizeT
		_拼接SizeT[0] = 索引->SizeT;
	//在此之后才可以读入各种变长成员，而在此之前不知道索引文件的大小所以不能读入

	std::vector<uint32_t>::const_iterator 块像素头 = 每块像素数向量.cbegin();
	uint32_t* 每块像素头 = 每块像素数;
	//第0块像素数就是真正的第0块像素数，而第1块实际上可能还是第0块（如果索引->SizeC>1）
	for (const uint32_t* const 每块像素尾 = 每块像素头 + 索引->每帧分块数; 每块像素头 < 每块像素尾; 块像素头 += 索引->SizeC)
		*(每块像素头++) = *块像素头;
	std::copy_n(通道颜色.get(), 索引->SizeC, i通道颜色);
	try
	{
		for (const uint16_t* const 指针 : 块指针)
			*(块偏移++) = (char*)指针 - 全局文件头;
	}
	catch (...)
	{
		throw Image5D::Exception::Index_block_offset_write_failure;
	}
	if (strcpy_s(索引->Z驱动单元类型, (节点 = 成像参数节点.child("commonparam:zDriveUnitType")) && (节点文本 = 节点.text()) ? 节点文本.get() : ""))
		throw Image5D::Exception::Z_driver_type_is_too_long;
	//不是每个通道都会有对应的PMT参数，电流检测通道就没有
	struct 电压设置
	{
		std::vector<Z设置<uint16_t>>分层设置;
		uint16_t 固定设置;
	};
	std::unordered_map<std::string, 电压设置>探测器电压;
	std::vector<Z设置<float>>激光设置;
	if (有Z)
	{
		const xml_node 亮度调整参数节点 = 成像参数节点.child("lsmparam:brightnessAdjustmentParam");
		if (亮度调整参数节点 && (节点 = 亮度调整参数节点.child("lsmparam:enable")) && (节点文本 = 节点.text()) && 节点文本.as_bool())
		{
			for (xml_node 节点 : 亮度调整参数节点.children("lsmparam:phaseBrightnessAdjustment"))
				if ((节点属性 = 节点.attribute("zDriveUnitType")) && !strcmp(节点属性.value(), 索引->Z驱动单元类型))
					for (const xml_node Z亮度节点 : 节点.children("lsmparam:zBrightnessAdjustment"))
					{
						if (!((节点 = Z亮度节点.child("lsmparam:relativeCoordinate")) && (节点文本 = 节点.text())))
							throw Image5D::Exception::Z_brightness_is_missing_relative_coordinates;
						const float 深度 = 节点文本.as_float();
						for (xml_node 节点 : Z亮度节点.children("lsmparam:pmtParam"))
							if ((节点属性 = 节点.attribute("detectorId")) && (节点 = 节点.child("lsmparam:voltage")) && (节点文本 = 节点.text()))
								探测器电压[节点属性.value()].分层设置.push_back({ 深度,(uint16_t)节点文本.as_uint() });
							else
								throw Image5D::Exception::The_Z_PMT_Settings_are_incomplete;
						if ((节点 = Z亮度节点.child("lsmparam:laserParam")) && (节点 = 节点.child("commonparam:transmissivity")) && (节点文本 = 节点.text()))
							激光设置.push_back({ 深度,节点文本.as_float() });
						else
							throw Image5D::Exception::Z_Laser_setup_is_incomplete;
					}
		}
	}
	for (xml_node 节点 : 成像参数节点.children("lsmparam:pmt"))
	{
		if (!(节点属性 = 节点.attribute("detectorId")))
			throw Image5D::Exception::PMT_lacks_detectorId;
		电压设置& PMT电压 = 探测器电压[节点属性.value()];
		if (PMT电压.分层设置.empty())
			if ((节点 = 节点.child("lsmparam:voltage")) && (节点文本 = 节点.text()))
				PMT电压.固定设置 = 节点文本.as_uint();
			else
				throw Image5D::Exception::PMT_voltage_not_defined;
	}
	if (有Z && 激光设置.size())
		深度参数生成(激光设置, Z起点, 索引->SizeZ, i激光透过率, 索引->Z步长);
	else if ((节点 = 成像参数节点.child("lsmparam:mainLaser")) && (节点 = 节点.child("commonparam:transmissivity")) && (节点文本 = 节点.text()))
		std::fill_n(i激光透过率, 索引->SizeZ, 节点文本.as_float());
	else
		throw Image5D::Exception::Missing_laser_parameters;
	uint16_t* 输出头 = i放大电压;
	for (const 通道设备& C : 通道设备向量)
	{
		const 电压设置& 通道电压 = 探测器电压[C.探测器ID];
		if (有Z && 通道电压.分层设置.size())
			深度参数生成(通道电压.分层设置, Z起点, 索引->SizeZ, 输出头, 索引->Z步长);
		else
			std::fill_n(输出头, 索引->SizeZ, 通道电压.固定设置);
		输出头 += 索引->SizeZ;
	}
	索引->哈希签名(文件大小);
}
Oir读入器::Oir读入器(LPCWSTR 头文件路径)
{
	const uint16_t 长度 = wcslen(头文件路径) - 4;
	//make_unique会对内存初始化，不用于分配POD数组
	const std::unique_ptr<wchar_t[]> 字符缓冲 = std::make_unique_for_overwrite<wchar_t[]>(长度 + 7);
	std::copy_n(头文件路径, 长度, 字符缓冲.get());
	wchar_t* const 分隔符位置 = 字符缓冲.get() + 长度;
	*分隔符位置 = L'_';
	wchar_t* const 编号位置 = 分隔符位置 + 1;
	文件列表.push_back(std::make_unique<文件控制块>(头文件路径));
	uint64_t 总映射空间 = 文件列表.back()->粒度大小;
	while (true)
	{
		//无限循环查找下一个文件，直到因文件不存在而出错被捕获break
		swprintf(编号位置, L"%05u", 文件列表.size());
		try
		{
			文件列表.push_back(std::make_unique<文件控制块>(字符缓冲.get()));
		}
		catch (const Win32异常&ex)
		{
			if (ex.Image5D异常 == Exception::File_opening_failed)[[likely]]
				break;
			else
				throw;
		}
		总映射空间 += 文件列表.back()->粒度大小;
	}
	void* 尾指针 = VirtualAlloc(nullptr, 总映射空间 + 分配粒度, MEM_RESERVE, PAGE_READONLY);
	VirtualFree(尾指针, 0, MEM_RELEASE);
	for (const std::unique_ptr<文件控制块>& 文件 : 文件列表)
	{
		文件->映射指针(尾指针);
		尾指针 = (char*)尾指针 + 文件->粒度大小;
	}
	wcscpy(分隔符位置, L".Oir索引");
	try
	{
		//直接throw;无法捕获
		索引文件 = std::make_unique<文件映射>(字符缓冲.get(), true);
		索引文件->映射指针(nullptr);
		索引 = (Oir索引*)索引文件->映射指针();
		if (sizeof(索引) > 索引文件->文件大小() || !索引->验证(索引文件->文件大小()))
			throw Image5D::Exception::Parsing_the_index_file_failed;
		uint64_t* 块偏移;
		索引->Get变长成员(i激光透过率, i通道颜色, i放大电压, 每块像素数, 块偏移, _拼接SizeT);
		const uint32_t 块总数 = uint32_t(索引->SizeZBC) * 索引->SizeT;
		if (!块总数)
			throw Image5D::Exception::Parsing_the_index_file_failed;
		if ((char*)(块偏移 + 块总数) > (char*)索引文件->映射指针() + 索引文件->文件大小())
			throw Image5D::Exception::Parsing_the_index_file_failed;
		const char* const 映射指针 = (char*)文件列表[0]->映射指针();
		if (映射指针 + 块偏移[块总数 - 1] > 尾指针)
			throw Image5D::Exception::Parsing_the_index_file_failed;
		for (const uint64_t* const 块偏移尾 = 块偏移 + 块总数; 块偏移 < 块偏移尾; ++块偏移)
		{
			const uint16_t* 指针 = (const uint16_t*)(映射指针 + *块偏移);
			if (指针 < 尾指针)
				块指针.push_back(指针);
			else
				throw Image5D::Exception::Parsing_the_index_file_failed;
		}
	}
	catch (Image5D::Exception ex)
	{
		if (ex == Image5D::Exception::The_library_version_is_lower_than_the_index_file_version)
			throw ex;
		创建新索引(字符缓冲.get());
	}
	catch (Image5D::Win32异常 ex)
	{
		if (ex.Win32错误码 == ERROR_FILE_NOT_FOUND)
			创建新索引(字符缓冲.get());
		else
			throw;
	}
	catch (...)
	{
		//throw Image5D异常(索引加载失败);
		创建新索引(字符缓冲.get());
	}
}
void Oir读入器::读入像素(uint16_t* 写出头TZ, uint32_t TStart, uint32_t TSize, uint8_t ZStart, uint8_t ZSize, uint8_t CStart, uint8_t CSize)const
{
	if (TStart + TSize > 索引->SizeT || ZStart + ZSize > 索引->SizeZ || CStart + CSize > 索引->SizeC)
		throw Image5D::Exception::Specified_dimension_out_of_bounds;
	const uint16_t* const* 读入头T = 块指针.data() + (TStart * 索引->SizeZ + ZStart) * 索引->每帧分块数 * 索引->SizeC + CStart;
	const uint16_t* const* const 读入尾T = 读入头T + TSize * 索引->SizeZBC;
	const uint16_t 读入ZBC = (uint16_t)ZSize * 索引->SizeBC;
	const uint32_t 写出CYX = uint32_t(CSize) * 索引->SizeYX;
	try
	{
		while (读入头T < 读入尾T)
		{
			const uint16_t* const* 读入头ZB = 读入头T;
			const uint16_t* const* 读入尾Z = 读入头ZB + 读入ZBC;
			while (读入头ZB < 读入尾Z)
			{
				const uint32_t* 块像素头 = 每块像素数;
				const uint16_t* const* 读入尾B = 读入头ZB + 索引->SizeBC;
				uint16_t* 写出头B = 写出头TZ;
				while (读入头ZB < 读入尾B)
				{
					const uint16_t* const* 读入头C = 读入头ZB;
					const uint16_t* const* 读入尾C = 读入头C + CSize;
					uint16_t* 写出头C = 写出头B;
					uint32_t 块像素数 = *块像素头;
					while (读入头C < 读入尾C)
					{
						std::copy_n(*读入头C, 块像素数, 写出头C);
						读入头C++;
						写出头C += 索引->SizeYX;
					}
					读入头ZB += 索引->SizeC;
					写出头B += 块像素数;
					块像素头++;
				}
				写出头TZ += 写出CYX;
			}
			读入头T += 索引->SizeZBC;
		}
	}
	catch (...)
	{
		throw Exception::Memory_copy_failed;
	}
}
void Oir读入器::读入像素(uint16_t* 写出头TZ, uint32_t TStart, uint32_t TSize, uint8_t ZStart, uint8_t ZSize)const
{
	if (TStart + TSize > 索引->SizeT || ZStart + ZSize > 索引->SizeZ)
		throw Exception::Specified_dimension_out_of_bounds;
	const uint16_t* const* 读入头T = 块指针.data() + (TStart * 索引->SizeZ + ZStart) * 索引->每帧分块数 * 索引->SizeC;
	const uint16_t* const* const 读入尾T = 读入头T + TSize * 索引->SizeZBC;
	const uint16_t 读入ZBC = (uint16_t)ZSize * 索引->SizeBC;
	try
	{
		while (读入头T < 读入尾T)
		{
			const uint16_t* const* 读入头ZB = 读入头T;
			const uint16_t* const* 读入尾Z = 读入头ZB + 读入ZBC;
			while (读入头ZB < 读入尾Z)
			{
				const uint32_t* 块像素头 = 每块像素数;
				const uint16_t* const* 读入尾B = 读入头ZB + 索引->SizeBC;
				uint16_t* 写出头B = 写出头TZ;
				while (读入头ZB < 读入尾B)
				{
					const uint16_t* const* 读入尾C = 读入头ZB + 索引->SizeC;
					uint16_t* 写出头C = 写出头B;
					uint32_t 块像素数 = *块像素头;
					while (读入头ZB < 读入尾C)
					{
						std::copy_n(*(读入头ZB++), 块像素数, 写出头C);
						写出头C += 索引->SizeYX;
					}
					写出头B += 块像素数;
					块像素头++;
				}
				写出头TZ += 索引->SizeCYX;
			}
			读入头T += 索引->SizeZBC;
		}
	}
	catch (...)
	{
		throw Exception::Memory_copy_failed;
	}
}
void Oir读入器::读入像素(uint16_t* 写出头, uint32_t TStart, uint32_t TSize, uint8_t C)const
{
	if (TStart + TSize > 索引->SizeT || C >= 索引->SizeC)
		throw Exception::Specified_dimension_out_of_bounds;
	const uint16_t* const* 读入头 = 块指针.data() + TStart * 索引->SizeZBC + C;
	const uint16_t* const* const 读入尾T = 读入头 + TSize * 索引->SizeZBC;
	try
	{
		while (读入头 < 读入尾T)
		{
			const uint16_t* const* 读入尾B = 读入头 + 索引->SizeBC;
			const uint32_t* 块像素头 = 每块像素数;
			while (读入头 < 读入尾B)
			{
				std::copy_n(*读入头, *块像素头, 写出头);
				读入头 += 索引->SizeC;
				写出头 += *块像素头;
				块像素头++;
			}
		}
	}
	catch (...)
	{
		throw Exception::Memory_copy_failed;
	}
}
void Oir读入器::读入像素(uint16_t* 写出头TZ, uint32_t TStart, uint32_t TSize)const
{
	if (TStart + TSize > 索引->SizeT)
		throw Exception::Specified_dimension_out_of_bounds;
	const uint16_t* const* 读入头 = 块指针.data() + TStart * 索引->SizeZBC;
	const uint16_t* const* const 读入尾T = 读入头 + TSize * 索引->SizeZBC;
	try
	{
		while (读入头 < 读入尾T)
		{
			const uint16_t* const* 读入尾B = 读入头 + 索引->SizeBC;
			uint16_t* 写出头B = 写出头TZ;
			const uint32_t* 块像素头 = 每块像素数;
			while (读入头 < 读入尾B)
			{
				const uint16_t* const* 读入尾C = 读入头 + 索引->SizeC;
				uint32_t 块像素数 = *块像素头;
				uint16_t* 写出头C = 写出头B;
				while (读入头 < 读入尾C)
				{
					std::copy_n(*读入头, 块像素数, 写出头C);
					读入头++;
					写出头C += 索引->SizeYX;
				}
				写出头B += 块像素数;
				块像素头++;
			}
			写出头TZ += 索引->SizeCYX;
		}
	}
	catch (...)
	{
		throw Exception::Memory_copy_failed;
	}
}
void Oir读入器::读入像素(uint16_t* 写出头TZ)const
{
	const uint16_t* const* 读入头 = 块指针.data();
	const uint16_t* const* const 读入尾T = 读入头 + 索引->SizeTZBC;
	try
	{
		while (读入头 < 读入尾T)
		{
			const uint16_t* const* 读入尾B = 读入头 + 索引->SizeBC;
			uint16_t* 写出头B = 写出头TZ;
			const uint32_t* 块像素头 = 每块像素数;
			while (读入头 < 读入尾B)
			{
				const uint16_t* const* 读入尾C = 读入头 + 索引->SizeC;
				uint32_t 块像素数 = *块像素头;
				uint16_t* 写出头C = 写出头B;
				while (读入头 < 读入尾C)
				{
					std::copy_n(*读入头, 块像素数, 写出头C);
					读入头++;
					写出头C += 索引->SizeYX;
				}
				写出头B += 块像素数;
				块像素头++;
			}
			写出头TZ += 索引->SizeCYX;
		}
	}
	catch (...)
	{
		throw Exception::Memory_copy_failed;
	}
}