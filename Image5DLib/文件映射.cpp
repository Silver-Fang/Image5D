#include "文件映射.h"
using namespace Image5D;
文件映射::文件映射(LPCWSTR 文件路径, bool 只读) :
	文件句柄(CreateFileW(文件路径, 只读 ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE, 只读 ? FILE_SHARE_READ | FILE_SHARE_WRITE : FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)), 只读(只读)
{
	if (文件句柄 == INVALID_HANDLE_VALUE)
		throw Win32异常{Exception::File_opening_failed, GetLastError()};
	//此映射有可能失败（文件大小为0）
	if (!(映射句柄 = CreateFileMapping(文件句柄, NULL, 只读 ? PAGE_READONLY : PAGE_READWRITE, 0, 0, NULL)))
	{
		Win32异常 异常{Exception::Memory_mapping_failed, GetLastError()};
		CloseHandle(文件句柄);
		throw 异常;
	}
	GetFileSizeEx(文件句柄, (LARGE_INTEGER*)&i文件大小);
}
文件映射::文件映射(LPCWSTR 文件路径, LONGLONG 文件大小) :
	文件句柄(CreateFileW(文件路径, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)), 只读(false), i文件大小(文件大小)
{
	if (文件句柄 == INVALID_HANDLE_VALUE)
		throw Win32异常{Exception::File_creation_failed, GetLastError()};
	LARGE_INTEGER 文件分位{ .QuadPart = 文件大小 };
	//先调整大小再映射并不能阻止未写入区域被重写入盘
	if (!(映射句柄 = CreateFileMapping(文件句柄, NULL, PAGE_READWRITE, 文件分位.HighPart, 文件分位.LowPart, NULL)))
	{
		Win32异常 异常{ Exception::Memory_mapping_failed, GetLastError() };
		CloseHandle(文件句柄);
		throw 异常;
	}
}
void 文件映射::映射指针(LPVOID 基地址)
{
	if (i映射指针)
		UnmapViewOfFile(i映射指针);
	if (基地址 = MapViewOfFileEx(映射句柄, 只读 ? FILE_MAP_READ : FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0, 基地址))
		i映射指针 = 基地址;
	else
	{
		Win32异常 异常{ Exception::Memory_mapping_failed, GetLastError() };
		if (i映射指针)
			MapViewOfFileEx(映射句柄, 只读 ? FILE_MAP_READ : FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0, i映射指针);
		throw 异常;
	}
}
void 文件映射::文件大小(LONGLONG 大小)
{
	UnmapViewOfFile(i映射指针);
	CloseHandle(映射句柄);
	LARGE_INTEGER 文件大小{ .QuadPart = 大小 };
	if (!(SetFilePointerEx(文件句柄, 文件大小, &文件大小, FILE_BEGIN) && SetEndOfFile(文件句柄)))
	{
		Win32异常 异常{Exception::Failed_to_set_the_file_pointer, GetLastError()};
		映射句柄 = CreateFileMapping(文件句柄, NULL, 只读 ? PAGE_READONLY : PAGE_READWRITE, 0, 0, NULL);
		if (i映射指针)
			i映射指针 = MapViewOfFileEx(映射句柄, 只读 ? FILE_MAP_READ : FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0, i映射指针);
		throw 异常;
	}
	if (!(映射句柄 = CreateFileMapping(文件句柄, NULL, PAGE_READWRITE, 0, 0, NULL)))
	{
		Win32异常 异常{ Exception::Memory_mapping_failed, GetLastError() };
		CloseHandle(文件句柄);
		throw 异常;
	}
	if (i映射指针)
		i映射指针 = MapViewOfFile(映射句柄, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	i文件大小 = 大小;
}
文件映射::~文件映射()noexcept
{
	UnmapViewOfFile(i映射指针);
	CloseHandle(映射句柄);
	CloseHandle(文件句柄);
}