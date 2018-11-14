// Copyright (c) 2016-2018 Easy2D - Nomango
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

#include "TaskManager.h"

namespace easy2d
{
	void TaskManager::AddTask(spTask const& task)
	{
		if (task)
		{
			auto iter = std::find(tasks_.begin(), tasks_.end(), task);
			if (iter == tasks_.end())
			{
				task->Reset();
				tasks_.push_back(task);
			}
		}
	}

	void TaskManager::StopTasks(const String& name)
	{
		for (const auto& task : tasks_)
		{
			if (task->GetName() == name)
			{
				task->Stop();
			}
		}
	}

	void TaskManager::StartTasks(const String& name)
	{
		for (const auto& task : tasks_)
		{
			if (task->GetName() == name)
			{
				task->Start();
			}
		}
	}

	void TaskManager::RemoveTasks(const String& name)
	{
		for (const auto& task : tasks_)
		{
			if (task->GetName() == name)
			{
				task->stopped_ = true;
			}
		}
	}

	void TaskManager::StopAllTasks()
	{
		for (const auto& task : tasks_)
		{
			task->Stop();
		}
	}

	void TaskManager::StartAllTasks()
	{
		for (const auto& task : tasks_)
		{
			task->Start();
		}
	}

	void TaskManager::RemoveAllTasks()
	{
		for (const auto& task : tasks_)
		{
			task->stopped_ = true;
		}
	}

	const Tasks & TaskManager::GetAllTasks() const
	{
		return tasks_;
	}

	void TaskManager::UpdateTasks(Duration const& dt)
	{
		if (tasks_.empty())
			return;

		std::vector<spTask> currTasks;
		currTasks.reserve(tasks_.size());
		std::copy_if(
			tasks_.begin(),
			tasks_.end(),
			std::back_inserter(currTasks),
			[](spTask const& task) { return !task->stopped_; }
		);

		// ��������������
		for (const auto& task : currTasks)
			task->Update(dt);

		// �������������
		for (auto iter = tasks_.begin(); iter != tasks_.end();)
		{
			if ((*iter)->stopped_)
			{
				iter = tasks_.erase(iter);
			}
			else
			{
				++iter;
			}
		}
	}
}