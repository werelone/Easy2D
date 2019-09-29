// Copyright (c) 2016-2018 Kiwano - Nomango
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "Resource.h"
#include "../base/Logger.h"

#include <iostream>
namespace kiwano
{
	Resource::Resource()
		: id_(0)
		, type_(nullptr)
	{

	}

	Resource::Resource(std::uint32_t id, const wchar_t* type)
		: id_(id)
		, type_(type)
	{
	}

	Resource::Data Resource::GetData() const
	{
		do
		{
			if (data_.buffer && data_.size)
			{
				break;
			}

			HRSRC res_info = FindResourceW(nullptr, MAKEINTRESOURCE(id_), type_);
			if (res_info == nullptr)
			{
				KGE_ERROR_LOG(L"FindResource failed");
				break;
			}

			HGLOBAL res_data = LoadResource(nullptr, res_info);
			if (res_data == nullptr)
			{
				KGE_ERROR_LOG(L"LoadResource failed");
				break;
			}

			DWORD size = SizeofResource(nullptr, res_info);
			if (size == 0)
			{
				KGE_ERROR_LOG(L"SizeofResource failed");
				break;
			}

			LPVOID buffer = LockResource(res_data);
			if (buffer == nullptr)
			{
				KGE_ERROR_LOG(L"LockResource failed");
				break;
			}

			data_.buffer = static_cast<void*>(buffer);
			data_.size = static_cast<std::uint32_t>(size);
		} while (0);

		return data_;
	}
}
