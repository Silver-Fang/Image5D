#pragma once
#include <mex.hpp>
#include <mexAdapter.hpp>
#include <algorithm>
#include "内存优化库.h"
using namespace matlab::data;
using namespace matlab::mex;
extern ArrayFactory 数组工厂;
template <typename T>
concept 数值 = std::is_arithmetic<T>::value;
//MATLAB转C++
//一般标量转换
template<typename T>
inline T 万能转码(Array& 输入)
{
	return TypedArray<T>(std::move(输入))[0];
}
//一般指针转换自uint64
template <typename T>
	requires std::is_pointer<T>::value
inline T 万能转码(Array& 输入)
{
	return (T)万能转码<size_t>(输入);
}
template<>
String 万能转码<String>(Array& 输入);
template<>
CharArray 万能转码<CharArray>(Array& 输入);
template<>
char* 万能转码<char*>(Array& 输入);
using 窄字符数组 = std::unique_ptr<char[], free删除器>;
template<typename T>
requires std::_Is_any_of_v<T,char[]>
inline 窄字符数组 万能转码(Array& 输入)
{
	return 窄字符数组(万能转码<char*>(输入));
}

//C++转MATLAB

template <typename T>
inline buffer_ptr_t<T> 万能转码(const T* 指针, UINT64 尺寸)noexcept
{
	buffer_ptr_t<T>缓冲 = 数组工厂.createBuffer<T>(尺寸);
	std::copy_n(指针, 尺寸, 缓冲.get());
	return 缓冲;
}

inline TypedArray<uint64_t>万能转码(const void*指针)
{
	return 数组工厂.createScalar((uint64_t)指针);
}

template <数值 T>
inline TypedArray<T> 万能转码(T 输入)
{
	return 数组工厂.createScalar(输入);
}

template<class T>
requires std::_Is_any_of_v<T,CharArray,MATLABString>
T 万能转码(const char*);

#define API声明(函数名) void 函数名(ArgumentList& outputs,ArgumentList& inputs)
#define API索引 constexpr void (*(API[]))(ArgumentList&, ArgumentList&) =
#define API调用 API[万能转码<UINT8>(inputs[0])](outputs, inputs);
class MexFunction :public Function
{
public:
	void operator()(ArgumentList outputs, ArgumentList inputs);
};
void 异常输出补全(ArgumentList& outputs);