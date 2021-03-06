#include "pch.h"
#include "Image5D.h"
void 检查文件句柄(HANDLE 文件句柄)
{
	if (文件句柄 == INVALID_HANDLE_VALUE)
	{
		const DWORD 错误代码 = GetLastError();
		switch (错误代码)
		{
		case ERROR_FILE_NOT_FOUND:
			throw Image5D异常(找不到指定的文件);
		case ERROR_PATH_NOT_FOUND:
			throw Image5D异常(找不到指定的路径);
		case ERROR_ACCESS_DENIED:
			throw Image5D异常(文件拒绝访问);
		case ERROR_SHARING_VIOLATION:
			throw Image5D异常(文件被另一个进程占用);
		default:
			throw Image5D异常(文件打开失败, 错误代码);
		}
	}
}
Image5D异常 检查映射句柄(HANDLE 映射句柄)noexcept
{
	if (映射句柄)
		return 成功异常;
	else
	{
		const DWORD 错误代码 = GetLastError();
		switch (错误代码)
		{
		case ERROR_DISK_FULL:
			return Image5D异常(磁盘已满);
		case ERROR_FILE_INVALID:
			return Image5D异常(文件大小为0);
		default:
			return Image5D异常(内存映射失败, 错误代码);
		}
	}
}
void 句柄构造(HANDLE 文件句柄, HANDLE& 映射句柄, LONGLONG& i文件大小, bool 只读)
{
	检查文件句柄(文件句柄);
	//此映射有可能失败（文件大小为0）
	const Image5D异常 异常 = 检查映射句柄(映射句柄 = CreateFileMapping(文件句柄, NULL, 只读 ? PAGE_READONLY : PAGE_READWRITE, 0, 0, NULL));
	if (异常.类型 != 操作成功)
	{
		CloseHandle(文件句柄);
		throw 异常;
	}
	GetFileSizeEx(文件句柄, (LARGE_INTEGER*)&i文件大小);
}
文件映射::文件映射(const char* 文件路径, bool 只读) :
	文件句柄(CreateFileA(文件路径, 只读 ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)), 只读(只读)
{
	句柄构造(文件句柄, 映射句柄, i文件大小, 只读);
}
文件映射::文件映射(const wchar_t* 文件路径, bool 只读) :
	文件句柄(CreateFileW(文件路径, 只读 ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)), 只读(只读)
{
	句柄构造(文件句柄, 映射句柄, i文件大小, 只读);
}
union 分位数
{
	struct
	{
		DWORD 低位;
		DWORD 高位;
	};
	LONGLONG 全位;
};
void 覆盖创建(HANDLE 文件句柄, HANDLE& 映射句柄, LONGLONG 文件大小)
{
	检查文件句柄(文件句柄);
	分位数 文件分位{ .全位 = 文件大小 };
	const Image5D异常 异常 = 检查映射句柄(映射句柄 = CreateFileMapping(文件句柄, NULL, PAGE_READWRITE, 文件分位.高位, 文件分位.低位, NULL));
	if (异常.类型 != 操作成功)
	{
		CloseHandle(文件句柄);
		throw 异常;
	}
}
文件映射::文件映射(LPCSTR 文件路径, LONGLONG 文件大小) :
	文件句柄(CreateFileA(文件路径, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)), 只读(false),i文件大小(文件大小)
{
	覆盖创建(文件句柄, 映射句柄, i文件大小);
}
文件映射::文件映射(LPCWSTR 文件路径, LONGLONG 文件大小) :
	文件句柄(CreateFileW(文件路径, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)), 只读(false),i文件大小(文件大小)
{
	覆盖创建(文件句柄, 映射句柄, i文件大小);
}
Image5D异常 文件映射::映射指针(LPVOID 基地址)noexcept
{
	if (i映射指针)
		UnmapViewOfFile(i映射指针);
	if (基地址 = MapViewOfFileEx(映射句柄, 只读 ? FILE_MAP_READ : FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0, 基地址))
		i映射指针 = 基地址;
	else
	{
		Image5D异常 异常(内存映射失败, GetLastError());
		if (i映射指针)
			MapViewOfFileEx(映射句柄, 只读 ? FILE_MAP_READ : FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0, i映射指针);
		return 异常;
	}
	return 成功异常;
}
Image5D异常 文件映射::文件大小(LONGLONG 大小)noexcept
{
	UnmapViewOfFile(i映射指针);
	CloseHandle(映射句柄);
	LARGE_INTEGER 文件大小{ .QuadPart = 大小 };
	if (!(SetFilePointerEx(文件句柄, 文件大小, &文件大小, FILE_BEGIN) && SetEndOfFile(文件句柄)))
	{
		Image5D异常 异常(文件指针设置失败, GetLastError());
		映射句柄 = CreateFileMapping(文件句柄, NULL, 只读 ? PAGE_READONLY : PAGE_READWRITE, 0, 0, NULL);
		if (i映射指针)
			i映射指针 = MapViewOfFileEx(映射句柄, 只读 ? FILE_MAP_READ : FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0, i映射指针);
		return 异常;
	}
	映射句柄 = CreateFileMapping(文件句柄, NULL,  PAGE_READWRITE, 0, 0, NULL);
	if (i映射指针)
		i映射指针 = MapViewOfFile(映射句柄, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	i文件大小 = 大小;
	return 成功异常;
}
文件映射::~文件映射()noexcept
{
	UnmapViewOfFile(i映射指针);
	CloseHandle(映射句柄);
	CloseHandle(文件句柄);
}
template<字符 T>
Image5D异常 文件映射::打开(const T* 文件路径, bool 只读, std::unique_ptr<文件映射>& 返回指针)noexcept
{
	try
	{
		返回指针.reset(new 文件映射(文件路径, 只读));
	}
	catch (Image5D异常& 异常)
	{
		return 异常;
	}
	return 成功异常;
}
template<字符 T>
Image5D异常 文件映射::创建(const T* 文件路径, LONGLONG 文件大小, std::unique_ptr<文件映射>& 返回指针)noexcept
{
	try
	{
		返回指针.reset(new 文件映射(文件路径, 文件大小));
	}
	catch (Image5D异常& 异常)
	{
		return 异常;
	}
	return 成功异常;
}