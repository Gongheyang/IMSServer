/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@文件    MySRand.cpp
 *@描述    随机数生成类实现

 *
 *@作者    GhY
 *@日期    2024年7月24日
 *@版本    v1.0.0
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
