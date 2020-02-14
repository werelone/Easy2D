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

#include <kiwano/core/Exception.h>
#include <kiwano/core/Logger.h>
#include <kiwano/core/event/WindowEvent.h>
#include <kiwano/platform/FileSystem.h>
#include <kiwano/platform/win32/WindowImpl.h>
#include <kiwano/render/ShapeSink.h>
#include <kiwano/render/DirectX/TextureRenderContextImpl.h>
#include <kiwano/render/DirectX/RendererImpl.h>

namespace kiwano
{

Renderer& Renderer::GetInstance()
{
    return RendererImpl::GetInstance();
}

RendererImpl& RendererImpl::GetInstance()
{
    static RendererImpl instance;
    return instance;
}

RendererImpl::RendererImpl()
{
    render_ctx_ = new RenderContextImpl;
}

void RendererImpl::SetupModule()
{
    KGE_SYS_LOG("Creating device resources");

    ThrowIfFailed(::CoInitialize(nullptr), "CoInitialize failed");

    HWND target_window = WindowImpl::GetInstance().GetHandle();
    output_size_   = Window::GetInstance().GetSize();

    d2d_res_ = nullptr;
    d3d_res_ = nullptr;

    HRESULT hr = target_window ? S_OK : E_FAIL;

    // Direct3D device resources
    if (SUCCEEDED(hr))
    {
        hr = ID3DDeviceResources::Create(&d3d_res_, target_window);

        // Direct2D device resources
        if (SUCCEEDED(hr))
        {
            hr = ID2DDeviceResources::Create(&d2d_res_, d3d_res_->GetDXGIDevice(), d3d_res_->GetDXGISwapChain());

            // Other device resources
            if (SUCCEEDED(hr))
            {
                hr = render_ctx_->CreateDeviceResources(d2d_res_->GetFactory(), d2d_res_->GetDeviceContext());
            }

            // FontFileLoader and FontCollectionLoader
            if (SUCCEEDED(hr))
            {
                hr = IFontCollectionLoader::Create(&font_collection_loader_);

                if (SUCCEEDED(hr))
                {
                    hr = d2d_res_->GetDWriteFactory()->RegisterFontCollectionLoader(font_collection_loader_.get());
                }
            }

            // ResourceFontFileLoader and ResourceFontCollectionLoader
            if (SUCCEEDED(hr))
            {
                hr = IResourceFontFileLoader::Create(&res_font_file_loader_);

                if (SUCCEEDED(hr))
                {
                    hr = d2d_res_->GetDWriteFactory()->RegisterFontFileLoader(res_font_file_loader_.get());
                }

                if (SUCCEEDED(hr))
                {
                    hr = IResourceFontCollectionLoader::Create(&res_font_collection_loader_,
                                                               res_font_file_loader_.get());

                    if (SUCCEEDED(hr))
                    {
                        hr = d2d_res_->GetDWriteFactory()->RegisterFontCollectionLoader(
                            res_font_collection_loader_.get());
                    }
                }
            }
        }
    }

    ThrowIfFailed(hr, "Create render resources failed");
}

void RendererImpl::DestroyModule()
{
    KGE_SYS_LOG("Destroying device resources");

    d2d_res_->GetDWriteFactory()->UnregisterFontFileLoader(res_font_file_loader_.get());
    res_font_file_loader_.reset();

    d2d_res_->GetDWriteFactory()->UnregisterFontCollectionLoader(res_font_collection_loader_.get());
    res_font_collection_loader_.reset();

    render_ctx_.reset();
    d2d_res_.reset();
    d3d_res_.reset();

    ::CoUninitialize();
}

void RendererImpl::BeginDraw()
{
    KGE_ASSERT(render_ctx_);

    render_ctx_->BeginDraw();
}

void RendererImpl::EndDraw()
{
    KGE_ASSERT(render_ctx_);

    render_ctx_->EndDraw();
}

void RendererImpl::Clear()
{
    KGE_ASSERT(d3d_res_);

    d3d_res_->ClearRenderTarget(clear_color_);
}

void RendererImpl::Present()
{
    KGE_ASSERT(d3d_res_);

    HRESULT hr = d3d_res_->Present(vsync_);

    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        // 如果 Direct3D 设备在执行过程中消失，将丢弃当前的设备相关资源
        hr = HandleDeviceLost();
    }

    ThrowIfFailed(hr, "Unexpected DXGI exception");
}

void RendererImpl::HandleEvent(Event* evt)
{
    if (evt->IsType<WindowResizedEvent>())
    {
        auto window_evt = dynamic_cast<WindowResizedEvent*>(evt);
        Resize(window_evt->width, window_evt->height);
    }
}

HRESULT RendererImpl::HandleDeviceLost()
{
    KGE_ASSERT(d3d_res_ && d2d_res_ && render_ctx_);

    HRESULT hr = d3d_res_->HandleDeviceLost();

    if (SUCCEEDED(hr))
    {
        hr = d2d_res_->HandleDeviceLost(d3d_res_->GetDXGIDevice(), d3d_res_->GetDXGISwapChain());
    }

    if (SUCCEEDED(hr))
    {
        hr = render_ctx_->CreateDeviceResources(d2d_res_->GetFactory(), d2d_res_->GetDeviceContext());
    }
    return hr;
}

void RendererImpl::CreateTexture(Texture& texture, String const& file_path)
{
    HRESULT hr = S_OK;
    if (!d2d_res_)
    {
        hr = E_UNEXPECTED;
    }

    if (!FileSystem::GetInstance().IsFileExists(file_path))
    {
        KGE_WARN("Texture file '%s' not found!", file_path.c_str());
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    if (SUCCEEDED(hr))
    {
        WideString full_path = MultiByteToWide(FileSystem::GetInstance().GetFullPathForFile(file_path));

        ComPtr<IWICBitmapDecoder> decoder;
        hr = d2d_res_->CreateBitmapDecoderFromFile(decoder, full_path.c_str());

        if (SUCCEEDED(hr))
        {
            ComPtr<IWICBitmapFrameDecode> source;
            hr = decoder->GetFrame(0, &source);

            if (SUCCEEDED(hr))
            {
                ComPtr<IWICFormatConverter> converter;
                hr = d2d_res_->CreateBitmapConverter(converter, source, GUID_WICPixelFormat32bppPBGRA,
                                                     WICBitmapDitherTypeNone, nullptr, 0.f,
                                                     WICBitmapPaletteTypeMedianCut);

                if (SUCCEEDED(hr))
                {
                    ComPtr<ID2D1Bitmap> bitmap;
                    hr = d2d_res_->CreateBitmapFromConverter(bitmap, nullptr, converter);

                    if (SUCCEEDED(hr))
                    {
                        texture.SetBitmap(bitmap);
                    }
                }
            }
        }
    }

    if (FAILED(hr))
    {
        ThrowIfFailed(hr, "Load texture failed");
    }
}

void RendererImpl::CreateTexture(Texture& texture, Resource const& resource)
{
    HRESULT hr = S_OK;
    if (!d2d_res_)
    {
        hr = E_UNEXPECTED;
    }

    if (SUCCEEDED(hr))
    {
        Resource::Data data = resource.GetData();

        hr = data ? S_OK : E_FAIL;

        if (SUCCEEDED(hr))
        {
            ComPtr<IWICBitmapDecoder> decoder;
            hr = d2d_res_->CreateBitmapDecoderFromResource(decoder, data.buffer, (DWORD)data.size);

            if (SUCCEEDED(hr))
            {
                ComPtr<IWICBitmapFrameDecode> source;
                hr = decoder->GetFrame(0, &source);

                if (SUCCEEDED(hr))
                {
                    ComPtr<IWICFormatConverter> converter;
                    hr = d2d_res_->CreateBitmapConverter(converter, source, GUID_WICPixelFormat32bppPBGRA,
                                                         WICBitmapDitherTypeNone, nullptr, 0.f,
                                                         WICBitmapPaletteTypeMedianCut);

                    if (SUCCEEDED(hr))
                    {
                        ComPtr<ID2D1Bitmap> bitmap;
                        hr = d2d_res_->CreateBitmapFromConverter(bitmap, nullptr, converter);

                        if (SUCCEEDED(hr))
                        {
                            texture.SetBitmap(bitmap);
                        }
                    }
                }
            }
        }
    }

    if (FAILED(hr))
    {
        KGE_WARN("Load texture failed with HRESULT of %08X!", hr);
    }
}

void RendererImpl::CreateGifImage(GifImage& gif, String const& file_path)
{
    HRESULT hr = S_OK;
    if (!d2d_res_)
    {
        hr = E_UNEXPECTED;
    }

    if (!FileSystem::GetInstance().IsFileExists(file_path))
    {
        KGE_WARN("Gif texture file '%s' not found!", file_path.c_str());
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    if (SUCCEEDED(hr))
    {
        WideString full_path = MultiByteToWide(FileSystem::GetInstance().GetFullPathForFile(file_path));

        ComPtr<IWICBitmapDecoder> decoder;
        hr = d2d_res_->CreateBitmapDecoderFromFile(decoder, full_path.c_str());

        if (SUCCEEDED(hr))
        {
            gif.SetDecoder(decoder);
        }
    }

    if (FAILED(hr))
    {
        KGE_WARN("Load GIF texture failed with HRESULT of %08X!", hr);
    }
}

void RendererImpl::CreateGifImage(GifImage& gif, Resource const& resource)
{
    HRESULT hr = S_OK;
    if (!d2d_res_)
    {
        hr = E_UNEXPECTED;
    }

    if (SUCCEEDED(hr))
    {
        Resource::Data data = resource.GetData();

        hr = data ? S_OK : E_FAIL;

        if (SUCCEEDED(hr))
        {
            ComPtr<IWICBitmapDecoder> decoder;
            hr = d2d_res_->CreateBitmapDecoderFromResource(decoder, data.buffer, (DWORD)data.size);

            if (SUCCEEDED(hr))
            {
                gif.SetDecoder(decoder);
            }
        }
    }

    if (FAILED(hr))
    {
        KGE_WARN("Load GIF texture failed with HRESULT of %08X!", hr);
    }
}

void RendererImpl::CreateGifImageFrame(GifImage::Frame& frame, GifImage const& gif, size_t frame_index)
{
    HRESULT hr = S_OK;
    if (!d2d_res_)
    {
        hr = E_UNEXPECTED;
    }

    if (gif.GetDecoder() == nullptr)
    {
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr))
    {
        ComPtr<IWICBitmapFrameDecode> wic_frame;

        HRESULT hr = gif.GetDecoder()->GetFrame(UINT(frame_index), &wic_frame);

        if (SUCCEEDED(hr))
        {
            ComPtr<IWICFormatConverter> converter;
            d2d_res_->CreateBitmapConverter(converter, wic_frame, GUID_WICPixelFormat32bppPBGRA,
                                            WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeCustom);

            if (SUCCEEDED(hr))
            {
                ComPtr<ID2D1Bitmap> raw_bitmap;
                hr = d2d_res_->CreateBitmapFromConverter(raw_bitmap, nullptr, converter);

                if (SUCCEEDED(hr))
                {
                    frame.texture = new Texture;
                    frame.texture->SetBitmap(raw_bitmap);
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            PROPVARIANT prop_val;
            PropVariantInit(&prop_val);

            // Get Metadata Query Reader from the frame
            ComPtr<IWICMetadataQueryReader> metadata_reader;
            hr = wic_frame->GetMetadataQueryReader(&metadata_reader);

            // Get the Metadata for the current frame
            if (SUCCEEDED(hr))
            {
                hr = metadata_reader->GetMetadataByName(L"/imgdesc/Left", &prop_val);
                if (SUCCEEDED(hr))
                {
                    hr = (prop_val.vt == VT_UI2 ? S_OK : E_FAIL);
                    if (SUCCEEDED(hr))
                    {
                        frame.rect.left_top.x = static_cast<float>(prop_val.uiVal);
                    }
                    PropVariantClear(&prop_val);
                }
            }

            if (SUCCEEDED(hr))
            {
                hr = metadata_reader->GetMetadataByName(L"/imgdesc/Top", &prop_val);
                if (SUCCEEDED(hr))
                {
                    hr = (prop_val.vt == VT_UI2 ? S_OK : E_FAIL);
                    if (SUCCEEDED(hr))
                    {
                        frame.rect.left_top.y = static_cast<float>(prop_val.uiVal);
                    }
                    PropVariantClear(&prop_val);
                }
            }

            if (SUCCEEDED(hr))
            {
                hr = metadata_reader->GetMetadataByName(L"/imgdesc/Width", &prop_val);
                if (SUCCEEDED(hr))
                {
                    hr = (prop_val.vt == VT_UI2 ? S_OK : E_FAIL);
                    if (SUCCEEDED(hr))
                    {
                        frame.rect.right_bottom.x = frame.rect.left_top.x + static_cast<float>(prop_val.uiVal);
                    }
                    PropVariantClear(&prop_val);
                }
            }

            if (SUCCEEDED(hr))
            {
                hr = metadata_reader->GetMetadataByName(L"/imgdesc/Height", &prop_val);
                if (SUCCEEDED(hr))
                {
                    hr = (prop_val.vt == VT_UI2 ? S_OK : E_FAIL);
                    if (SUCCEEDED(hr))
                    {
                        frame.rect.right_bottom.y = frame.rect.left_top.y + static_cast<float>(prop_val.uiVal);
                    }
                    PropVariantClear(&prop_val);
                }
            }

            if (SUCCEEDED(hr))
            {
                hr = metadata_reader->GetMetadataByName(L"/grctlext/Delay", &prop_val);

                if (SUCCEEDED(hr))
                {
                    hr = (prop_val.vt == VT_UI2 ? S_OK : E_FAIL);

                    if (SUCCEEDED(hr))
                    {
                        uint32_t udelay = 0;

                        hr = UIntMult(prop_val.uiVal, 10, &udelay);
                        if (SUCCEEDED(hr))
                        {
                            frame.delay.SetMilliseconds(static_cast<long>(udelay));
                        }
                    }
                    PropVariantClear(&prop_val);
                }
                else
                {
                    frame.delay = 0;
                }
            }

            if (SUCCEEDED(hr))
            {
                hr = metadata_reader->GetMetadataByName(L"/grctlext/Disposal", &prop_val);

                if (SUCCEEDED(hr))
                {
                    hr = (prop_val.vt == VT_UI1) ? S_OK : E_FAIL;
                    if (SUCCEEDED(hr))
                    {
                        frame.disposal_type = GifImage::DisposalType(prop_val.bVal);
                    }
                    ::PropVariantClear(&prop_val);
                }
                else
                {
                    frame.disposal_type = GifImage::DisposalType::Unknown;
                }
            }

            ::PropVariantClear(&prop_val);
        }
    }

    if (FAILED(hr))
    {
        KGE_WARN("Load GIF frame failed with HRESULT of %08X!", hr);
    }
}

void RendererImpl::CreateFontCollection(Font& font, String const& file_path)
{
    HRESULT hr = S_OK;
    if (!d2d_res_)
    {
        hr = E_UNEXPECTED;
    }

    if (SUCCEEDED(hr))
    {
        if (!FileSystem::GetInstance().IsFileExists(file_path))
        {
            KGE_WARN("Font file '%s' not found!", file_path.c_str());
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        }
    }

    if (SUCCEEDED(hr))
    {
        LPVOID   key       = nullptr;
        uint32_t key_size  = 0;
        String   full_path = FileSystem::GetInstance().GetFullPathForFile(file_path);

        hr = font_collection_loader_->AddFilePaths({ full_path }, &key, &key_size);

        if (SUCCEEDED(hr))
        {
            ComPtr<IDWriteFontCollection> font_collection;
            hr = d2d_res_->GetDWriteFactory()->CreateCustomFontCollection(font_collection_loader_.get(), key, key_size,
                                                                          &font_collection);

            if (SUCCEEDED(hr))
            {
                font.SetCollection(font_collection);
            }
        }
    }

    ThrowIfFailed(hr, "Create font collection failed");
}

void RendererImpl::CreateFontCollection(Font& font, Resource const& res)
{
    HRESULT hr = S_OK;
    if (!d2d_res_)
    {
        hr = E_UNEXPECTED;
    }

    if (SUCCEEDED(hr))
    {
        LPVOID   key      = nullptr;
        uint32_t key_size = 0;

        hr = res_font_collection_loader_->AddResources(Vector<Resource>{ res }, &key, &key_size);

        if (SUCCEEDED(hr))
        {
            ComPtr<IDWriteFontCollection> font_collection;
            hr = d2d_res_->GetDWriteFactory()->CreateCustomFontCollection(res_font_collection_loader_.get(), key,
                                                                          key_size, &font_collection);

            if (SUCCEEDED(hr))
            {
                font.SetCollection(font_collection);
            }
        }
    }

    ThrowIfFailed(hr, "Create font collection failed");
}

void RendererImpl::CreateTextFormat(TextLayout& layout)
{
    HRESULT hr = S_OK;
    if (!d2d_res_)
    {
        hr = E_UNEXPECTED;
    }

    ComPtr<IDWriteTextFormat> output;
    if (SUCCEEDED(hr))
    {
        const TextStyle& style = layout.GetStyle();

        hr = d2d_res_->CreateTextFormat(
            output, MultiByteToWide(style.font_family).c_str(), style.font ? style.font->GetCollection() : nullptr,
            DWRITE_FONT_WEIGHT(style.font_weight), style.italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL, style.font_size);
    }

    if (SUCCEEDED(hr))
    {
        layout.SetTextFormat(output);
    }

    ThrowIfFailed(hr, "Create text format failed");
}

void RendererImpl::CreateTextLayout(TextLayout& layout)
{
    HRESULT hr = S_OK;
    if (!d2d_res_)
    {
        hr = E_UNEXPECTED;
    }

    ComPtr<IDWriteTextLayout> output;
    if (SUCCEEDED(hr))
    {
        WideString text = MultiByteToWide(layout.GetText());

        hr = d2d_res_->CreateTextLayout(output, text.c_str(), text.length(), layout.GetTextFormat());
    }

    if (SUCCEEDED(hr))
    {
        layout.SetTextLayout(output);
    }

    ThrowIfFailed(hr, "Create text layout failed");
}

void RendererImpl::CreateLineShape(Shape& shape, Point const& begin_pos, Point const& end_pos)
{
    HRESULT hr = S_OK;
    if (!d2d_res_)
    {
        hr = E_UNEXPECTED;
    }

    ComPtr<ID2D1PathGeometry> path_geo;
    ComPtr<ID2D1GeometrySink> path_sink;
    if (SUCCEEDED(hr))
    {
        hr = d2d_res_->GetFactory()->CreatePathGeometry(&path_geo);
    }

    if (SUCCEEDED(hr))
    {
        hr = path_geo->Open(&path_sink);
    }

    if (SUCCEEDED(hr))
    {
        path_sink->BeginFigure(DX::ConvertToPoint2F(begin_pos), D2D1_FIGURE_BEGIN_FILLED);
        path_sink->AddLine(DX::ConvertToPoint2F(end_pos));
        path_sink->EndFigure(D2D1_FIGURE_END_OPEN);
        hr = path_sink->Close();
    }

    if (SUCCEEDED(hr))
    {
        shape.SetGeometry(path_geo);
    }

    ThrowIfFailed(hr, "Create ID2D1PathGeometry failed");
}

void RendererImpl::CreateRectShape(Shape& shape, Rect const& rect)
{
    HRESULT hr = S_OK;
    if (!d2d_res_)
    {
        hr = E_UNEXPECTED;
    }

    ComPtr<ID2D1RectangleGeometry> output;
    if (SUCCEEDED(hr))
    {
        hr = d2d_res_->GetFactory()->CreateRectangleGeometry(DX::ConvertToRectF(rect), &output);
    }

    if (SUCCEEDED(hr))
    {
        shape.SetGeometry(output);
    }

    ThrowIfFailed(hr, "Create ID2D1RectangleGeometry failed");
}

void RendererImpl::CreateRoundedRectShape(Shape& shape, Rect const& rect, Vec2 const& radius)
{
    HRESULT hr = S_OK;
    if (!d2d_res_)
    {
        hr = E_UNEXPECTED;
    }

    ComPtr<ID2D1RoundedRectangleGeometry> output;
    if (SUCCEEDED(hr))
    {
        hr = d2d_res_->GetFactory()->CreateRoundedRectangleGeometry(
            D2D1::RoundedRect(DX::ConvertToRectF(rect), radius.x, radius.y), &output);
    }

    if (SUCCEEDED(hr))
    {
        shape.SetGeometry(output);
    }

    ThrowIfFailed(hr, "Create ID2D1RoundedRectangleGeometry failed");
}

void RendererImpl::CreateEllipseShape(Shape& shape, Point const& center, Vec2 const& radius)
{
    HRESULT hr = S_OK;
    if (!d2d_res_)
    {
        hr = E_UNEXPECTED;
    }

    ComPtr<ID2D1EllipseGeometry> output;
    if (SUCCEEDED(hr))
    {
        hr = d2d_res_->GetFactory()->CreateEllipseGeometry(
            D2D1::Ellipse(DX::ConvertToPoint2F(center), radius.x, radius.y), &output);
    }

    if (SUCCEEDED(hr))
    {
        shape.SetGeometry(output);
    }

    ThrowIfFailed(hr, "Create ID2D1EllipseGeometry failed");
}

void RendererImpl::CreateShapeSink(ShapeSink& sink)
{
    HRESULT hr = S_OK;
    if (!d2d_res_)
    {
        hr = E_UNEXPECTED;
    }

    ComPtr<ID2D1PathGeometry> output;
    if (SUCCEEDED(hr))
    {
        hr = d2d_res_->GetFactory()->CreatePathGeometry(&output);
    }

    if (SUCCEEDED(hr))
    {
        sink.SetPathGeometry(output);
    }

    ThrowIfFailed(hr, "Create ID2D1PathGeometry failed");
}

void RendererImpl::CreateBrush(Brush& brush, Color const& color)
{
    HRESULT hr = S_OK;
    if (!d2d_res_)
    {
        hr = E_UNEXPECTED;
    }

    if (SUCCEEDED(hr))
    {
        ComPtr<ID2D1SolidColorBrush> solid_brush;

        if (brush.GetType() == Brush::Type::SolidColor && brush.GetBrush())
        {
            hr = brush.GetBrush()->QueryInterface(&solid_brush);
            if (SUCCEEDED(hr))
            {
                solid_brush->SetColor(DX::ConvertToColorF(color));
            }
        }
        else
        {
            hr = d2d_res_->GetDeviceContext()->CreateSolidColorBrush(DX::ConvertToColorF(color), &solid_brush);

            if (SUCCEEDED(hr))
            {
                brush.SetBrush(solid_brush, Brush::Type::SolidColor);
            }
        }
    }

    ThrowIfFailed(hr, "Create ID2D1SolidBrush failed");
}

void RendererImpl::CreateBrush(Brush& brush, LinearGradientStyle const& style)
{
    HRESULT hr = S_OK;
    if (!d2d_res_)
    {
        hr = E_UNEXPECTED;
    }

    if (SUCCEEDED(hr))
    {
        ComPtr<ID2D1GradientStopCollection> collection;
        hr = d2d_res_->GetDeviceContext()->CreateGradientStopCollection(
            reinterpret_cast<const D2D1_GRADIENT_STOP*>(&style.stops[0]), UINT32(style.stops.size()), D2D1_GAMMA_2_2,
            D2D1_EXTEND_MODE(style.extend_mode), &collection);

        if (SUCCEEDED(hr))
        {
            ComPtr<ID2D1LinearGradientBrush> output;
            hr = d2d_res_->GetDeviceContext()->CreateLinearGradientBrush(
                D2D1::LinearGradientBrushProperties(DX::ConvertToPoint2F(style.begin), DX::ConvertToPoint2F(style.end)),
                collection.get(), &output);

            if (SUCCEEDED(hr))
            {
                brush.SetBrush(output, Brush::Type::LinearGradient);
            }
        }
    }

    ThrowIfFailed(hr, "Create ID2D1LinearGradientBrush failed");
}

void RendererImpl::CreateBrush(Brush& brush, RadialGradientStyle const& style)
{
    HRESULT hr = S_OK;
    if (!d2d_res_)
    {
        hr = E_UNEXPECTED;
    }

    if (SUCCEEDED(hr))
    {
        ComPtr<ID2D1GradientStopCollection> collection;
        hr = d2d_res_->GetDeviceContext()->CreateGradientStopCollection(
            reinterpret_cast<const D2D1_GRADIENT_STOP*>(&style.stops[0]), UINT32(style.stops.size()), D2D1_GAMMA_2_2,
            D2D1_EXTEND_MODE(style.extend_mode), &collection);

        if (SUCCEEDED(hr))
        {
            ComPtr<ID2D1RadialGradientBrush> output;
            hr = d2d_res_->GetDeviceContext()->CreateRadialGradientBrush(
                D2D1::RadialGradientBrushProperties(DX::ConvertToPoint2F(style.center),
                                                    DX::ConvertToPoint2F(style.offset), style.radius.x, style.radius.y),
                collection.get(), &output);

            if (SUCCEEDED(hr))
            {
                brush.SetBrush(output, Brush::Type::RadialGradient);
            }
        }
    }

    ThrowIfFailed(hr, "Create ID2D1RadialGradientBrush failed");
}

void RendererImpl::CreateStrokeStyle(StrokeStyle& stroke_style, CapStyle cap, LineJoinStyle line_join,
                                 const float* dash_array, size_t dash_size, float dash_offset)
{
    HRESULT hr = S_OK;
    if (!d2d_res_)
    {
        hr = E_UNEXPECTED;
    }

    if (SUCCEEDED(hr))
    {
        D2D1_STROKE_STYLE_PROPERTIES style =
            D2D1::StrokeStyleProperties(D2D1_CAP_STYLE(cap), D2D1_CAP_STYLE(cap), D2D1_CAP_STYLE(cap),
                                        D2D1_LINE_JOIN(line_join), 10.0f, D2D1_DASH_STYLE_CUSTOM, dash_offset);

        ComPtr<ID2D1StrokeStyle> output;
        hr = d2d_res_->GetFactory()->CreateStrokeStyle(style, dash_array, dash_size, &output);

        if (SUCCEEDED(hr))
        {
            stroke_style.SetStrokeStyle(output);
        }
    }

    ThrowIfFailed(hr, "Create ID2D1StrokeStyle failed");
}

TextureRenderContextPtr RendererImpl::CreateTextureRenderContext(const Size* desired_size)
{
    TextureRenderContextImplPtr ptr = new TextureRenderContextImpl;

    HRESULT hr = S_OK;
    if (!d2d_res_)
    {
        hr = E_UNEXPECTED;
    }

    if (SUCCEEDED(hr))
    {
        ComPtr<ID2D1BitmapRenderTarget> bitmap_rt;

        if (desired_size)
        {
            hr = d2d_res_->GetDeviceContext()->CreateCompatibleRenderTarget(DX::ConvertToSizeF(*desired_size),
                                                                            &bitmap_rt);
        }
        else
        {
            hr = d2d_res_->GetDeviceContext()->CreateCompatibleRenderTarget(&bitmap_rt);
        }

        if (SUCCEEDED(hr))
        {
            hr = ptr->CreateDeviceResources(d2d_res_->GetFactory(), bitmap_rt);
        }

        if (SUCCEEDED(hr))
        {
            ptr->bitmap_rt_ = bitmap_rt;
        }
    }

    if (SUCCEEDED(hr))
        return ptr;

    return nullptr;
}

void RendererImpl::Resize(uint32_t width, uint32_t height)
{
    HRESULT hr = S_OK;

    if (!d3d_res_)
        hr = E_UNEXPECTED;

    if (SUCCEEDED(hr))
    {
        output_size_.x = static_cast<float>(width);
        output_size_.y = static_cast<float>(height);

        hr = d3d_res_->SetLogicalSize(output_size_);
    }

    if (SUCCEEDED(hr))
    {
        hr = d2d_res_->SetLogicalSize(output_size_.x, output_size_.y);
    }

    if (SUCCEEDED(hr))
    {
        render_ctx_->Resize(output_size_);
    }

    ThrowIfFailed(hr, "Resize render target failed");
}

}  // namespace kiwano
