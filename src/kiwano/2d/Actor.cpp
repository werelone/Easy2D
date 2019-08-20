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

#include "Actor.h"
#include "Stage.h"
#include "../base/Logger.h"
#include "../renderer/Renderer.h"

namespace kiwano
{
	namespace
	{
		Float32 default_anchor_x = 0.f;
		Float32 default_anchor_y = 0.f;
	}

	void Actor::SetDefaultAnchor(Float32 anchor_x, Float32 anchor_y)
	{
		default_anchor_x = anchor_x;
		default_anchor_y = anchor_y;
	}

	Actor::Actor()
		: visible_(true)
		, update_pausing_(false)
		, hover_(false)
		, pressed_(false)
		, responsible_(false)
		, dirty_transform_(false)
		, dirty_transform_inverse_(false)
		, cascade_opacity_(false)
		, show_border_(false)
		, is_fast_transform_(true)
		, parent_(nullptr)
		, stage_(nullptr)
		, hash_name_(0)
		, z_order_(0)
		, opacity_(1.f)
		, displayed_opacity_(1.f)
		, anchor_(default_anchor_x, default_anchor_y)
	{
	}

	void Actor::Update(Duration dt)
	{
		if (!update_pausing_)
		{
			UpdateActions(this, dt);
			UpdateTimers(dt);

			if (cb_update_)
				cb_update_(dt);

			OnUpdate(dt);
		}

		if (!children_.empty())
		{
			ActorPtr next;
			for (auto child = children_.first_item(); child; child = next)
			{
				next = child->next_item();
				child->Update(dt);
			}
		}
	}

	void Actor::Render(RenderTarget* rt)
	{
		if (!visible_)
			return;

		UpdateTransform();

		if (children_.empty())
		{
			OnRender(rt);
		}
		else
		{
			// render children those are less than 0 in Z-Order
			Actor* child = children_.first_item().get();
			while (child)
			{
				if (child->GetZOrder() >= 0)
					break;

				child->Render(rt);
				child = child->next_item().get();
			}

			OnRender(rt);

			while (child)
			{
				child->Render(rt);
				child = child->next_item().get();
			}
		}
	}

	void Actor::PrepareRender(RenderTarget* rt)
	{
		rt->SetTransform(transform_matrix_);
		rt->SetOpacity(displayed_opacity_);
	}

	void Actor::RenderBorder(RenderTarget* rt)
	{
		if (show_border_ && !size_.IsOrigin())
		{
            Rect bounds = GetBounds();

			rt->SetTransform(transform_matrix_);
			rt->FillRectangle(bounds, Color(Color::Red, .4f));
			rt->DrawRectangle(bounds, Color(Color::Red, .8f), 2.f);
		}

		for (auto child = children_.first_item(); child; child = child->next_item())
		{
			child->RenderBorder(rt);
		}
	}

	void Actor::Dispatch(Event& evt)
	{
		if (!visible_)
			return;

		ActorPtr prev;
		for (auto child = children_.last_item(); child; child = prev)
		{
			prev = child->prev_item();
			child->Dispatch(evt);
		}

		if (responsible_ && MouseEvent::Check(evt.type))
		{
			if (evt.type == Event::MouseMove)
			{
				if (!evt.target && ContainsPoint(Point{ evt.mouse.x, evt.mouse.y }))
				{
					evt.target = this;

					if (!hover_)
					{
						hover_ = true;

						Event hover = evt;
						hover.type = Event::MouseHover;
						EventDispatcher::Dispatch(hover);
					}
				}
				else if (hover_)
				{
					hover_ = false;
					pressed_ = false;

					Event out = evt;
					out.target = this;
					out.type = Event::MouseOut;
					EventDispatcher::Dispatch(out);
				}
			}

			if (evt.type == Event::MouseBtnDown && hover_)
			{
				pressed_ = true;
				evt.target = this;
			}

			if (evt.type == Event::MouseBtnUp && pressed_)
			{
				pressed_ = false;
				evt.target = this;

				Event click = evt;
				click.type = Event::Click;
				EventDispatcher::Dispatch(click);
			}
		}

		EventDispatcher::Dispatch(evt);
	}

	Matrix3x2 const & Actor::GetTransformMatrix()  const
	{
		UpdateTransform();
		return transform_matrix_;
	}

	Matrix3x2 const & Actor::GetTransformInverseMatrix()  const
	{
		UpdateTransform();
		if (dirty_transform_inverse_)
		{
			transform_matrix_inverse_ = transform_matrix_.Invert();
			dirty_transform_inverse_ = false;
		}
		return transform_matrix_inverse_;
	}

	void Actor::UpdateTransform() const
	{
		if (!dirty_transform_)
			return;

		dirty_transform_ = false;
		dirty_transform_inverse_ = true;

		if (is_fast_transform_)
		{
			transform_matrix_ = Matrix3x2::Translation(transform_.position);
		}
		else
		{
			// matrix multiplication is optimized by expression template
			transform_matrix_ = transform_.ToMatrix();
		}

		transform_matrix_.Translate(Point{ -size_.x * anchor_.x, -size_.y * anchor_.y });

		if (parent_)
		{
			transform_matrix_ *= parent_->transform_matrix_;
		}

		// update children's transform
		for (Actor* child = children_.first_item().get(); child; child = child->next_item().get())
			child->dirty_transform_ = true;
	}

	void Actor::UpdateOpacity()
	{
		if (parent_ && parent_->IsCascadeOpacityEnabled())
		{
			displayed_opacity_ = opacity_ * parent_->displayed_opacity_;
		}
		else
		{
			displayed_opacity_ = opacity_;
		}

		for (Actor* child = children_.first_item().get(); child; child = child->next_item().get())
		{
			child->UpdateOpacity();
		}
	}

	void Actor::SetStage(Stage* stage)
	{
		if (stage && stage_ != stage)
		{
			stage_ = stage;
			for (Actor* child = children_.first_item().get(); child; child = child->next_item().get())
			{
				child->stage_ = stage;
			}
		}
	}

	void Actor::Reorder()
	{
		if (parent_)
		{
			ActorPtr me = this;

			parent_->children_.remove(me);

			Actor* sibling = parent_->children_.last_item().get();

			if (sibling && sibling->GetZOrder() > z_order_)
			{
				sibling = sibling->prev_item().get();
				while (sibling)
				{
					if (sibling->GetZOrder() <= z_order_)
						break;
					sibling = sibling->prev_item().get();
				}
			}

			if (sibling)
			{
				parent_->children_.insert_after(me, sibling);
			}
			else
			{
				parent_->children_.push_front(me);
			}
		}
	}

	void Actor::SetZOrder(Int32 zorder)
	{
		if (z_order_ != zorder)
		{
			z_order_ = zorder;
			Reorder();
		}
	}

	void Actor::SetOpacity(Float32 opacity)
	{
		if (opacity_ == opacity)
			return;

		displayed_opacity_ = opacity_ = std::min(std::max(opacity, 0.f), 1.f);
		UpdateOpacity();
	}

	void Actor::SetCascadeOpacityEnabled(bool enabled)
	{
		if (cascade_opacity_ == enabled)
			return;

		cascade_opacity_ = enabled;
		UpdateOpacity();
	}

	void Actor::SetAnchor(Vec2 const& anchor)
	{
		if (anchor_ == anchor)
			return;

		anchor_ = anchor;
		dirty_transform_ = true;
	}

	void Actor::SetWidth(Float32 width)
	{
		SetSize(Size{ width, size_.y });
	}

	void Actor::SetHeight(Float32 height)
	{
		SetSize(Size{ size_.x, height });
	}

	void Actor::SetSize(Size const& size)
	{
		if (size_ == size)
			return;

		size_ = size;
		dirty_transform_ = true;
	}

	void Actor::SetTransform(Transform const& transform)
	{
		transform_ = transform;
		dirty_transform_ = true;
		is_fast_transform_ = false;
	}

	void Actor::SetVisible(bool val)
	{
		visible_ = val;
	}

	void Actor::SetName(String const& name)
	{
		if (!IsName(name))
		{
			ObjectBase::SetName(name);
			hash_name_ = std::hash<String>{}(name);
		}
	}

	void Actor::SetPosition(const Point & pos)
	{
		if (transform_.position == pos)
			return;

		transform_.position = pos;
		dirty_transform_ = true;
	}

	void Actor::SetPositionX(Float32 x)
	{
		SetPosition(Point{ x, transform_.position.y });
	}

	void Actor::SetPositionY(Float32 y)
	{
		SetPosition(Point{ transform_.position.x, y });
	}

	void Actor::Move(Vec2 const& v)
	{
		this->SetPosition(Point{ transform_.position.x + v.x, transform_.position.y + v.y });
	}

	void Actor::SetScale(Vec2 const& scale)
	{
		if (transform_.scale == scale)
			return;

		transform_.scale = scale;
		dirty_transform_ = true;
		is_fast_transform_ = false;
	}

	void Actor::SetSkew(Vec2 const& skew)
	{
		if (transform_.skew == skew)
			return;

		transform_.skew = skew;
		dirty_transform_ = true;
		is_fast_transform_ = false;
	}

	void Actor::SetRotation(Float32 angle)
	{
		if (transform_.rotation == angle)
			return;

		transform_.rotation = angle;
		dirty_transform_ = true;
		is_fast_transform_ = false;
	}

	void Actor::AddChild(ActorPtr child)
	{
		KGE_ASSERT(child && "Actor::AddChild failed, NULL pointer exception");

		if (child)
		{
#ifdef KGE_DEBUG

			if (child->parent_)
				KGE_ERROR_LOG(L"The actor to be added already has a parent");

			for (Actor* parent = parent_; parent; parent = parent->parent_)
				if (parent == child)
					KGE_ERROR_LOG(L"A actor cannot be its own parent");

#endif // KGE_DEBUG

			children_.push_back(child);
			child->parent_ = this;
			child->SetStage(this->stage_);
			child->dirty_transform_ = true;
			child->UpdateOpacity();
			child->Reorder();
		}
	}

	void Actor::AddChildren(Vector<ActorPtr> const& children)
	{
		for (const auto& actor : children)
		{
			this->AddChild(actor);
		}
	}

	Rect Actor::GetBounds() const
	{
		return Rect{ Point{}, size_ };
	}

	Rect Actor::GetBoundingBox() const
	{
		return GetTransformMatrix().Transform(GetBounds());
	}

	Vector<ActorPtr> Actor::GetChildren(String const& name) const
	{
		Vector<ActorPtr> children;
		UInt32 hash_code = std::hash<String>{}(name);

		for (Actor* child = children_.first_item().get(); child; child = child->next_item().get())
		{
			if (child->hash_name_ == hash_code && child->IsName(name))
			{
				children.push_back(child);
			}
		}
		return children;
	}

	ActorPtr Actor::GetChild(String const& name) const
	{
		UInt32 hash_code = std::hash<String>{}(name);

		for (Actor* child = children_.first_item().get(); child; child = child->next_item().get())
		{
			if (child->hash_name_ == hash_code && child->IsName(name))
			{
				return child;
			}
		}
		return nullptr;
	}

	Actor::Children const & Actor::GetChildren() const
	{
		return children_;
	}

	void Actor::RemoveFromParent()
	{
		if (parent_)
		{
			parent_->RemoveChild(this);
		}
	}

	void Actor::RemoveChild(ActorPtr child)
	{
		RemoveChild(child.get());
	}

	void Actor::RemoveChild(Actor * child)
	{
		KGE_ASSERT(child && "Actor::RemoveChild failed, NULL pointer exception");

		if (children_.empty())
			return;

		if (child)
		{
			child->parent_ = nullptr;
			if (child->stage_) child->SetStage(nullptr);
			children_.remove(ActorPtr(child));
		}
	}

	void Actor::RemoveChildren(String const& child_name)
	{
		if (children_.empty())
		{
			return;
		}

		UInt32 hash_code = std::hash<String>{}(child_name);

		Actor* next;
		for (Actor* child = children_.first_item().get(); child; child = next)
		{
			next = child->next_item().get();

			if (child->hash_name_ == hash_code && child->IsName(child_name))
			{
				RemoveChild(child);
			}
		}
	}

	void Actor::RemoveAllChildren()
	{
		children_.clear();
	}

	void Actor::SetResponsible(bool enable)
	{
		responsible_ = enable;
	}

	bool Actor::ContainsPoint(const Point& point) const
	{
		if (size_.x == 0.f || size_.y == 0.f)
			return false;

		Point local = GetTransformInverseMatrix().Transform(point);
		return GetBounds().ContainsPoint(local);
	}

}