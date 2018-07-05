#include "..\e2daction.h"
#include "..\e2dnode.h"

e2d::Animate::Animate() 
	: _frameIndex(0)
	, _animation(nullptr)
{
}

e2d::Animate::Animate(Animation * animation)
	: _frameIndex(0)
	, _animation(nullptr)
{
	this->setAnimation(animation);
}

e2d::Animate::~Animate()
{
}

e2d::Animation * e2d::Animate::getAnimation() const
{
	return _animation;
}

void e2d::Animate::setAnimation(Animation * animation)
{
	if (animation && animation != _animation)
	{
		GC::safeRelease(_animation);
		_animation = animation;
		_animation->retain();
	}
}

void e2d::Animate::_init()
{
	Action::_init();

	auto target = dynamic_cast<Sprite*>(_target);
	if (target && _animation)
	{
		target->open(_animation->getFrames()[_frameIndex]);
		++_frameIndex;
	}
}

void e2d::Animate::_update()
{
	Action::_update();

	if (!_animation)
	{
		this->stop();
		return;
	}

	while ((Time::getTotalTime() - _last) >= _animation->getInterval())
	{
		auto& frames = _animation->getFrames();
		auto target = dynamic_cast<Sprite*>(_target);

		if (target)
		{
			target->open(frames[_frameIndex]);
		}

		_last += _animation->getInterval();
		++_frameIndex;

		if (_frameIndex == frames.size())
		{
			this->stop();
			break;
		}
	}
}

void e2d::Animate::_resetTime()
{
	Action::_resetTime();
	_last = Time::getTotalTime();
}

void e2d::Animate::reset()
{
	Action::reset();
	_frameIndex = 0;
}

void e2d::Animate::onDestroy()
{
	Action::onDestroy();
	GC::safeRelease(_animation);
}

e2d::Animate * e2d::Animate::clone() const
{
	if (_animation)
	{
		return new (std::nothrow) Animate(_animation);
	}
	return nullptr;
}

e2d::Animate * e2d::Animate::reverse() const
{
	if (_animation)
	{
		auto animation = _animation->reverse();
		if (animation)
		{
			return new (std::nothrow) Animate(animation);
		}
	}
	return nullptr;
}
