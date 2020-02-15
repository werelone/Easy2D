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

#include <kiwano/render/Renderer.h>
#include <kiwano/render/Texture.h>

namespace kiwano
{

InterpolationMode Texture::default_interpolation_mode_ = InterpolationMode::Linear;

TexturePtr Texture::Create(String const& file_path)
{
    TexturePtr ptr = new (std::nothrow) Texture;
    if (ptr)
    {
        if (!ptr->Load(file_path))
            return nullptr;
    }
    return ptr;
}

TexturePtr Texture::Create(Resource const& res)
{
    TexturePtr ptr = new (std::nothrow) Texture;
    if (ptr)
    {
        if (!ptr->Load(res))
            return nullptr;
    }
    return ptr;
}

Texture::Texture()
    : interpolation_mode_(default_interpolation_mode_)
{
}

Texture::~Texture() {}

bool Texture::Load(String const& file_path)
{
    Renderer::GetInstance().CreateTexture(*this, file_path);
    return IsValid();
}

bool Texture::Load(Resource const& res)
{
    Renderer::GetInstance().CreateTexture(*this, res);
    return IsValid();
}

float Texture::GetWidth() const
{
    return size_.x;
}

float Texture::GetHeight() const
{
    return size_.y;
}

Size Texture::GetSize() const
{
    return size_;
}

uint32_t Texture::GetWidthInPixels() const
{
    return size_in_pixels_.x;
}

uint32_t Texture::GetHeightInPixels() const
{
    return size_in_pixels_.y;
}

math::Vec2T<uint32_t> Texture::GetSizeInPixels() const
{
    return size_in_pixels_;
}

InterpolationMode Texture::GetBitmapInterpolationMode() const
{
    return interpolation_mode_;
}

void Texture::CopyFrom(TexturePtr copy_from)
{
#if KGE_RENDER_ENGINE == KGE_RENDER_ENGINE_DIRECTX
    if (IsValid() && copy_from)
    {
        HRESULT hr = bitmap_->CopyFromBitmap(nullptr, copy_from->GetBitmap().Get(), nullptr);

        KGE_THROW_IF_FAILED(hr, "Copy texture data failed");
    }
#else
    return;  // not supported
#endif
}

void Texture::CopyFrom(TexturePtr copy_from, Rect const& src_rect, Point const& dest_point)
{
#if KGE_RENDER_ENGINE == KGE_RENDER_ENGINE_DIRECTX
    if (IsValid() && copy_from)
    {
        HRESULT hr = bitmap_->CopyFromBitmap(
            &D2D1::Point2U(uint32_t(dest_point.x), uint32_t(dest_point.y)), copy_from->GetBitmap().Get(),
            &D2D1::RectU(uint32_t(src_rect.GetLeft()), uint32_t(src_rect.GetTop()), uint32_t(src_rect.GetRight()),
                         uint32_t(src_rect.GetBottom())));

        KGE_THROW_IF_FAILED(hr, "Copy texture data failed");
    }
#else
    return;  // not supported
#endif
}

void Texture::SetInterpolationMode(InterpolationMode mode)
{
    interpolation_mode_ = mode;
    switch (mode)
    {
    case InterpolationMode::Linear:
        break;
    case InterpolationMode::Nearest:
        break;
    default:
        break;
    }
}

void Texture::SetDefaultInterpolationMode(InterpolationMode mode)
{
    default_interpolation_mode_ = mode;
}

InterpolationMode Texture::GetDefaultInterpolationMode()
{
    return default_interpolation_mode_;
}

}  // namespace kiwano
