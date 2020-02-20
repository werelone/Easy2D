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

#include <kiwano/2d/ShapeActor.h>
#include <kiwano/core/Logger.h>
#include <kiwano/render/Renderer.h>

namespace kiwano
{

ShapeActorPtr ShapeActor::Create(ShapePtr shape)
{
    ShapeActorPtr ptr = memory::New<ShapeActor>();
    if (ptr)
    {
        ptr->SetShape(shape);
    }
    return ptr;
}

ShapeActorPtr ShapeActor::Create(ShapePtr shape, const Color& fill_color, const Color& stroke_color)
{
    ShapeActorPtr ptr = ShapeActor::Create(shape);
    if (ptr)
    {
        ptr->SetFillColor(fill_color);
        ptr->SetStrokeColor(stroke_color);
    }
    return ptr;
}

ShapeActorPtr ShapeActor::Create(ShapePtr shape, BrushPtr fill_brush, BrushPtr stroke_brush)
{
    ShapeActorPtr ptr = ShapeActor::Create(shape);
    if (ptr)
    {
        ptr->SetFillBrush(fill_brush);
        ptr->SetStrokeBrush(stroke_brush);
    }
    return ptr;
}

ShapeActor::ShapeActor()
{
}

ShapeActor::~ShapeActor() {}

Rect ShapeActor::GetBounds() const
{
    return bounds_;
}

Rect ShapeActor::GetBoundingBox() const
{
    if (!shape_)
        return Rect{};

    return shape_->GetBoundingBox(GetTransformMatrix());
}

bool ShapeActor::ContainsPoint(const Point& point) const
{
    if (!shape_)
        return false;

    return shape_->ContainsPoint(point, &GetTransformMatrix());
}

void ShapeActor::SetShape(ShapePtr shape)
{
    shape_ = shape;
    if (shape_)
    {
        bounds_ = shape_->GetBoundingBox();
        SetSize(bounds_.GetSize());
    }
    else
    {
        bounds_ = Rect{};
        SetSize(0.f, 0.f);
    }
}

void ShapeActor::OnRender(RenderContext& ctx)
{
    if (shape_)
    {
        if (stroke_brush_)
        {
            ctx.SetCurrentBrush(stroke_brush_);
            ctx.SetCurrentStrokeStyle(stroke_style_);
            ctx.DrawShape(*shape_);
        }

        if (fill_brush_)
        {
            ctx.SetCurrentBrush(fill_brush_);
            ctx.FillShape(*shape_);
        }
    }
}

bool ShapeActor::CheckVisibility(RenderContext& ctx) const
{
    return shape_ && Actor::CheckVisibility(ctx);
}

//-------------------------------------------------------
// LineActor
//-------------------------------------------------------

LineActorPtr LineActor::Create(const Point& begin, const Point& end)
{
    LineActorPtr ptr = memory::New<LineActor>();
    if (ptr)
    {
        ptr->SetLine(begin, end);
    }
    return ptr;
}

LineActor::LineActor() {}

LineActor::~LineActor() {}

void LineActor::SetLine(const Point& begin, const Point& end)
{
    if (begin_ != begin || end_ != end)
    {
        begin_ = begin;
        end_   = end;
        SetShape(Shape::CreateLine(begin, end));
    }
}

//-------------------------------------------------------
// RectActor
//-------------------------------------------------------

RectActorPtr RectActor::Create(const Size& size)
{
    RectActorPtr ptr = memory::New<RectActor>();
    if (ptr)
    {
        ptr->SetRectSize(size);
    }
    return ptr;
}

RectActor::RectActor() {}

RectActor::~RectActor() {}

void RectActor::SetRectSize(const Size& size)
{
    if (size != rect_size_)
    {
        rect_size_ = size;
        SetShape(Shape::CreateRect(Rect{ Point{}, size }));
    }
}

//-------------------------------------------------------
// RoundedRectActor
//-------------------------------------------------------

RoundedRectActorPtr RoundedRectActor::Create(const Size& size, const Vec2& radius)
{
    RoundedRectActorPtr ptr = memory::New<RoundedRectActor>();
    if (ptr)
    {
        ptr->SetRoundedRect(size, radius);
    }
    return ptr;
}

RoundedRectActor::RoundedRectActor() {}

RoundedRectActor::~RoundedRectActor() {}

void RoundedRectActor::SetRadius(const Vec2& radius)
{
    SetRoundedRect(GetSize(), radius);
}

void RoundedRectActor::SetRectSize(const Size& size)
{
    SetRoundedRect(size, radius_);
}

void RoundedRectActor::SetRoundedRect(const Size& size, const Vec2& radius)
{
    if (rect_size_ != size || radius_ != radius)
    {
        rect_size_ = size;
        radius_    = radius;
        SetShape(Shape::CreateRoundedRect(Rect{ Point{}, size }, radius));
    }
}

//-------------------------------------------------------
// CircleActor
//-------------------------------------------------------

CircleActorPtr CircleActor::Create(float radius)
{
    CircleActorPtr ptr = memory::New<CircleActor>();
    if (ptr)
    {
        ptr->SetRadius(radius);
    }
    return ptr;
}

CircleActor::CircleActor()
    : radius_(0.f)
{
}

CircleActor::~CircleActor() {}

void CircleActor::SetRadius(float radius)
{
    if (radius_ != radius)
    {
        radius_ = radius;
        SetShape(Shape::CreateCircle(Point{ radius, radius }, radius));
    }
}

//-------------------------------------------------------
// EllipseActor
//-------------------------------------------------------

EllipseActorPtr EllipseActor::Create(const Vec2& radius)
{
    EllipseActorPtr ptr = memory::New<EllipseActor>();
    if (ptr)
    {
        ptr->SetRadius(radius);
    }
    return ptr;
}

EllipseActor::EllipseActor() {}

EllipseActor::~EllipseActor() {}

void EllipseActor::SetRadius(const Vec2& radius)
{
    if (radius_ != radius)
    {
        radius_ = radius;
        SetShape(Shape::CreateEllipse(radius, radius));
    }
}

//-------------------------------------------------------
// PolygonActor
//-------------------------------------------------------

PolygonActorPtr PolygonActor::Create(const Vector<Point>& points)
{
    PolygonActorPtr ptr = memory::New<PolygonActor>();
    if (ptr)
    {
        ptr->SetVertices(points);
    }
    return ptr;
}

PolygonActor::PolygonActor() {}

PolygonActor::~PolygonActor() {}

void PolygonActor::SetVertices(const Vector<Point>& points)
{
    if (points.size() > 1)
    {
        ShapeMakerPtr maker = ShapeMaker::Create();
        maker->BeginPath(points[0]);
        maker->AddLines(&points[1], points.size() - 1);
        maker->EndPath(true);

        SetShape(maker->GetShape());
    }
}

}  // namespace kiwano
