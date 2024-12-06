/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@�ļ�    MySRand.h
 *@����    ���������������
 *
 *@����    GhY
 *@����    2024��7��24��
 *@�汾    v1.0.0
 *
 ****************************************************/
#ifndef __MYSRAND_H__
#define __MYSRAND_H__
#include <string>


/*
 *@����:    �����������
 *@����:    GhY
 *@����:    2024/07/24
 *@��ʷ:
 */
class MySRand
{
public:
    MySRand();
    ~MySRand();

    /*
     *@desc       ��ȡ�����
     *@param:     length ���������
     *@return     �����
     *@author     GhY
     *@date       2024/07/17
     *@version    v1.0.0
     *@history:
     */
    std::string getNumber(int length = 6);

};

#endif // !__MYSRAND_H__

