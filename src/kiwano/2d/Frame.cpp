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

#include "Frame.h"
#include "../renderer/ImageCache.h"

namespace kiwano
{
	Frame::Frame()
	{
	}

	Frame::Frame(String const& file_path)
	{
		Load(file_path);
	}

	Frame::Frame(Resource const& res)
	{
		Load(res);
	}

	Frame::Frame(Image const& image)
	{
		SetImage(image);
	}

	bool Frame::Load(String const& file_path)
	{
		Image image = ImageCache::GetInstance()->AddOrGetImage(file_path);
		if (image.IsValid())
		{
			SetImage(image);
			return true;
		}
		return false;
	}

	bool Frame::Load(Resource const& res)
	{
		Image image = ImageCache::GetInstance()->AddOrGetImage(res);
		if (image.IsValid())
		{
			SetImage(image);
			return true;
		}
		return false;
	}

	void Frame::SetCropRect(Rect const& crop_rect)
	{
		if (image_.IsValid())
		{
			auto bitmap_size = image_.GetSize();
			crop_rect_.left_top.x = std::min(std::max(crop_rect.left_top.x, 0.f), bitmap_size.x);
			crop_rect_.left_top.y = std::min(std::max(crop_rect.left_top.y, 0.f), bitmap_size.y);
			crop_rect_.right_bottom.x = std::min(std::max(crop_rect.right_bottom.x, 0.f), bitmap_size.x);
			crop_rect_.right_bottom.y = std::min(std::max(crop_rect.right_bottom.y, 0.f), bitmap_size.y);
		}
	}

	void Frame::SetImage(Image const& image)
	{
		image_ = image;
		if (image_.IsValid())
		{
			crop_rect_.left_top.x = crop_rect_.left_top.y = 0;
			crop_rect_.right_bottom.x = image_.GetWidth();
			crop_rect_.right_bottom.y = image_.GetHeight();
		}
	}
}
