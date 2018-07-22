#include "..\e2dbase.h"
#include "..\e2dtool.h"
#include "..\e2dmanager.h"

using namespace e2d;

e2d::autorelease_t const e2d::autorelease = e2d::autorelease_t();

void * operator new(size_t size, e2d::autorelease_t const &) E2D_NOEXCEPT
{
	void* p = ::operator new(size, std::nothrow);
	if (p)
	{
		GC::getInstance()->autorelease(static_cast<Ref*>(p));
	}
	return p;
}

void operator delete(void * block, e2d::autorelease_t const &) E2D_NOEXCEPT
{
	::operator delete (block, std::nothrow);
}


// GC 机制，用于销毁所有单例
GC GC::_instance;

e2d::GC::GC()
	: _notifyed(false)
	, _cleanup(false)
	, _pool()
{
}

e2d::GC::~GC()
{
	// 删除所有对象
	this->clear();
	// 清除图片缓存
	Image::clearCache();
	// 删除所有单例
	Game::destroyInstance();
	Renderer::destroyInstance();
	Input::destroyInstance();
	Window::destroyInstance();
	Timer::destroyInstance();
	Player::destroyInstance();
	SceneManager::destroyInstance();
	ActionManager::destroyInstance();
	CollisionManager::destroyInstance();
}


void e2d::GC::flush()
{
	if (!_notifyed)
		return;

	_notifyed = false;
	for (auto iter = _pool.begin(); iter != _pool.end();)
	{
		if ((*iter)->getRefCount() <= 0)
		{
			delete (*iter);
			iter = _pool.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

void e2d::GC::clear()
{
	_cleanup = true;

	SceneManager::getInstance()->clear();
	Timer::getInstance()->clearAllTasks();
	ActionManager::getInstance()->clearAll();

	for (auto ref : _pool)
	{
		delete ref;
	}
	_pool.clear();
	_cleanup = false;
}

e2d::GC * e2d::GC::getInstance()
{
	return &_instance;
}

void e2d::GC::autorelease(Ref * ref)
{
	if (ref)
	{
		_pool.insert(ref);
	}
}

void e2d::GC::safeRelease(Ref* ref)
{
	if (_cleanup)
		return;

	if (ref)
	{
		auto iter = _pool.find(ref);
		if (iter != _pool.end())
		{
			(*iter)->release();
			_notifyed = true;
		}
	}
}
