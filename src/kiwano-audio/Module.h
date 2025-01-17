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
#include <kiwano-audio/Sound.h>
#include <kiwano-audio/Transcoder.h>
#include <kiwano/core/Common.h>
#include <kiwano/base/Module.h>
#include <xaudio2.h>

namespace kiwano
{
namespace audio
{

/**
 * \~chinese
 * \defgroup Audio 音频模块
 */

/**
 * \addtogroup Audio
 * @{
 */

/**
 * \~chinese
 * @brief 音频模块
 */
class KGE_API Module
    : public Singleton<Module>
    , public kiwano::Module
{
    friend Singleton<Module>;

public:
    /// \~chinese
    /// @brief 开启音频设备
    void Open();

    /// \~chinese
    /// @brief 关闭音频设备
    void Close();

    /// \~chinese
    /// @brief 注册解码器
    /// @param ext 文件类型（如：ogg），* 为默认解码器
    /// @param transcoder 解码器
    void RegisterTranscoder(StringView ext, RefPtr<Transcoder> transcoder);

    /// \~chinese
    /// @brief 获取解码器
    RefPtr<Transcoder> GetTranscoder(StringView ext);

    /// \~chinese
    /// @brief 解码音频
    /// @param file_path 本地音频文件路径
    RefPtr<AudioData> Decode(StringView file_path);

    /// \~chinese
    /// @brief 解码音频
    /// @param res 音频资源
    /// @param ext 音频类型，决定了使用何种解码器
    RefPtr<AudioData> Decode(const Resource& res, StringView ext = "");

    /// \~chinese
    /// @brief 创建音频
    bool CreateSound(Sound& sound, RefPtr<AudioData> data);

public:
    void SetupModule() override;

    void DestroyModule() override;

    ~Module();

private:
    Module();

private:
    IXAudio2*               x_audio2_;
    IXAudio2MasteringVoice* mastering_voice_;

    UnorderedMap<String, RefPtr<Transcoder>> registered_transcoders_;
};

/** @} */

}  // namespace audio
}  // namespace kiwano
