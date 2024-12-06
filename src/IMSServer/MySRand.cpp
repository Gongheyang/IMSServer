/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@�ļ�    MySRand.cpp
 *@����    �����������ʵ��

 *
 *@����    GhY
 *@����    2024��7��24��
 *@�汾    v1.0.0
 *
 ****************************************************/
#include "MySRand.h"
#include <stdlib.h>
#include <time.h>



MySRand::MySRand()
{
    srand((unsigned int)time(NULL));
}


MySRand::~MySRand()
{
}

std::string MySRand::getNumber(int length /*= 6*/)
{
    std::string retStr = "";
    while (length--) {
        int num = rand() % 9;
        if (num == 0) {
            num = rand() % 9;
        }
        retStr.append(std::to_string(num));
    }
    return retStr;
}
