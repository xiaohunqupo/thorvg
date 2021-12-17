/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd. All rights reserved.

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef _TVG_LOTTIE_LOADER_H_
#define _TVG_LOTTIE_LOADER_H_

#include "tvgTaskScheduler.h"
#include "rlottie.h"
#include "rlottiecommon.h"

class LottieLoader : public LoadModule, public Task
{
private:
    unique_ptr<rlottie::Animation> mLottieAnimation;
    unique_ptr<Scene> root;

    unique_ptr<Scene> sceneBuilder(const LOTLayerNode* lotRoot);
    void clear();
    bool header();

public:
    ~LottieLoader();

    using LoadModule::open;
    bool open(const string& path) override;
    bool open(const char* data, uint32_t size, bool copy) override;
    bool read() override;
    bool close() override;
    bool resize(Paint* paint, float w, float h) override;

    unique_ptr<Paint> paint() override;
    void run(unsigned tid) override;
};

#endif //_TVG_LOTTIE_LOADER_H_