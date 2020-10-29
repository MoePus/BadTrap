/*
 *  Copyright (c) 2020 MoePus
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.
 */

#include <iostream>
#include <windows.h>

extern "C" void Glitch();

int filter(unsigned int code, struct _EXCEPTION_POINTERS* ep, bool& bDetected, int& singleStepCount)
{
    if (code != EXCEPTION_SINGLE_STEP)
    {
        bDetected = true;
        return EXCEPTION_CONTINUE_SEARCH;
    }

    singleStepCount++;
    if ((size_t)ep->ExceptionRecord->ExceptionAddress != (size_t)Glitch + 11)
    {
        bDetected = true;
        return EXCEPTION_EXECUTE_HANDLER;
    }

    bool bIsRaisedBySingleStep = ep->ContextRecord->Dr6 & (1 << 14);
    bool bIsRaisedByDr0 = ep->ContextRecord->Dr6 & 1;
    if (!bIsRaisedBySingleStep || !bIsRaisedByDr0)
    {
        bDetected = true;
    }
    return EXCEPTION_EXECUTE_HANDLER;
}


bool DoDetection()
{
    bool bDetected = 0;
    int singleStepCount = 0;
    CONTEXT ctx{};
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
    GetThreadContext(GetCurrentThread(), &ctx);
    ctx.Dr7 = 1;
    SetThreadContext(GetCurrentThread(), &ctx);
    __try
    {
        Glitch();
    }
    __except (filter(GetExceptionCode(), GetExceptionInformation(), bDetected, singleStepCount))
    {
        if (singleStepCount != 1)
        {
            bDetected = 1;
        }
    }
    return bDetected;
}

int main()
{
    std::cout << "bIsVmOrVT: " << std::boolalpha << DoDetection() << std::endl;
}
