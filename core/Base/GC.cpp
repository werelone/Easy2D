#include "..\e2dbase.h"
#include "..\e2dtool.h"
#include "..\e2dmanager.h"

// GC 机制，用于自动销毁单例
e2d::GC e2d::GC::_instance;

e2d::GC::GC()
	: _notifyed(false)
	, _pool()
{
}

e2d::GC::~GC()
{
	// 删除所有单例
	Game::destroyInstance();
	Renderer::destroyInstance();
	Input::destroyInstance();
	Window::destroyInstance();
	Timer::destroyInstance();
	Player::destroyInstance();
	SceneManager::destroyInstance();
	ActionManager::destroyInstance();
	ColliderManager::destroyInstance();
}


// GC 释放池的实现机制：
// Object 类中的引用计数（_refCount）在一定程度上防止了内存泄漏
// 它记录了对象被使用的次数，当计数为 0 时，GC 会自动释放这个对象
// 所有的 Object 对象都应在被使用时（例如 Text 添加到了场景中）
// 调用 retain 函数保证该对象不被删除，并在不再使用时调用 release 函数
void e2d::GC::update()
{
	if (!_notifyed)
		return;

	_notifyed = false;
	for (auto iter = _pool.begin(); iter != _pool.end();)
	{
		if ((*iter)->getRefCount() <= 0)
		{
			(*iter)->onDestroy();
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
	for (auto pObj : _pool)
	{
		delete pObj;
	}
	_pool.clear();
}

void e2d::GC::addObject(e2d::Object * object)
{
	if (object)
	{
		_pool.insert(object);
	}
}

e2d::GC * e2d::GC::getInstance()
{
	return &_instance;
}

void e2d::GC::notify()
{
	_notifyed = true;
}
