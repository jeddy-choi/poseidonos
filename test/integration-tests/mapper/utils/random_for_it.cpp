/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "test/integration-tests/mapper/utils/random_for_it.h"

#include <iostream>
#include <random>

namespace pos
{
uint32_t
RandomForUT::_GenRandomUINT(void)
{
    std::random_device rndDevice;
    std::mt19937 engine(rndDevice());
    std::uniform_int_distribution<unsigned int> dist(0, UINT32_MAX);

    return dist(engine);
}

uint32_t
RandomForUT::_GetRandomSection(uint32_t min, uint32_t max)
{
    return min + (_GenRandomUINT() % (max + 1 - min));
}

void
RandomForUT::_FillRandomSection(uint32_t min, uint32_t max, std::vector<uint32_t>& testData, int dataCnt)
{
    std::random_device rndDevice;
    std::mt19937 engine(rndDevice());
    std::uniform_int_distribution<uint32_t> dist(min, max);

    testData.clear();
    for (int i = 0; i < dataCnt; ++i)
    {
        testData.push_back(dist(engine));
    }
}

void
RandomForUT::_PrintData(std::string name, std::vector<uint32_t>& data)
{
    std::cout << name << ": ";
    for (auto d : data)
    {
        std::cout << d << " ";
    }
    std::cout << std::endl;
}

}   // namespace pos
