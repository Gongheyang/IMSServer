/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@文件    MySRand.h
 *@描述    随机数生成类声明
 *
 *@作者    GhY
 *@日期    2024年7月24日
 *@版本    v1.0.0
 *
 ****************************************************/
#ifndef __MYSRAND_H__
#define __MYSRAND_H__
#include <string>


/*
 *@描述:    生成随机数类
 *@作者:    GhY
 *@日期:    2024/07/24
 *@历史:
 */
class MySRand
{
public:
    MySRand();
    ~MySRand();

    /*
     *@desc       获取随机数
     *@param:     length 随机数长度
     *@return     随机数
     *@author     GhY
     *@date       2024/07/17
     *@version    v1.0.0
     *@history:
     */
    std::string getNumber(int length = 6);

};

#endif // !__MYSRAND_H__

