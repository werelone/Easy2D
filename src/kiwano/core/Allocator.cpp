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

#include <kiwano/core/Allocator.h>

namespace kiwano
{
namespace memory
{

MemoryAllocator* current_allocator_ = GetGlobalAllocator();

MemoryAllocator* GetAllocator()
{
    return current_allocator_;
}

void SetAllocator(MemoryAllocator* allocator)
{
    KGE_ASSERT(allocator);
    current_allocator_ = allocator;
}

GlobalAllocator* GetGlobalAllocator()
{
    static GlobalAllocator global_allocator;
    return &global_allocator;
}

void* GlobalAllocator::Alloc(size_t size)
{
    return ::malloc(size);
}

void GlobalAllocator::Free(void* ptr)
{
    ::free(ptr);
}

}  // namespace memory
}  // namespace kiwano
