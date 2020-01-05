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

#pragma once
#include <kiwano/core/event/Event.h>

namespace kiwano
{
	/**
	* \addtogroup Events
	* @{
	*/

	/// \~chinese
	/// @brief �����¼�
	class KGE_API WindowEvent
		: public Event
	{
	};

	/// \~chinese
	/// @brief �����ƶ��¼�
	class KGE_API WindowMovedEvent
		: public Event
	{
	public:
		int x;			///< �������Ͻ� x ����
		int y;			///< �������Ͻ� y ����

		WindowMovedEvent();
	};

	/// \~chinese
	/// @brief ���ڴ�С�仯�¼�
	class KGE_API WindowResizedEvent
		: public Event
	{
	public:
		int width;		///< ���ڿ���
		int height;		///< ���ڸ߶�

		WindowResizedEvent();
	};

	/// \~chinese
	/// @brief ���ڽ���仯�¼�
	class KGE_API WindowFocusChangedEvent
		: public Event
	{
	public:
		bool focus;		///< �Ƿ��ȡ������

		WindowFocusChangedEvent();
	};

	/// \~chinese
	/// @brief ���ڱ�������¼�
	class KGE_API WindowTitleChangedEvent
		: public Event
	{
	public:
		String title;	///< ����

		WindowTitleChangedEvent();
	};

	/// \~chinese
	/// @brief ���ڹر��¼�
	class KGE_API WindowClosedEvent
		: public Event
	{
	public:
		WindowClosedEvent();
	};

	/** @} */

	template <>
	struct IsEventType<WindowEvent>
	{
		inline bool operator()(const Event& evt) const
		{
			return evt.GetType() == KGE_EVENT(WindowMovedEvent)
				|| evt.GetType() == KGE_EVENT(WindowResizedEvent)
				|| evt.GetType() == KGE_EVENT(WindowFocusChangedEvent)
				|| evt.GetType() == KGE_EVENT(WindowTitleChangedEvent)
				|| evt.GetType() == KGE_EVENT(WindowClosedEvent);
		}
	};

}