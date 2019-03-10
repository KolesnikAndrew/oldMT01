/*
 * OverflowFile.h
 *
 *  Created on: 31 окт. 2014
 *      Author: rtem
 */

#ifndef OVERFLOWFILE_H_
#define OVERFLOWFILE_H_

#include <sys/sysinfo.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <PostToServer.h>
#include <vector>
//#include <fstream>
//#include <algorithm>
#include <iterator>

using namespace std;

/**
 * @brief класс слежения за размером архива
 */
class OverflowFile
{
public:
    OverflowFile();
    virtual ~OverflowFile();
    /**
     * @fn Overflow
     * @brief Сбор статистики свободной памяти и запуск резки файлов
     * @details
     * @param
     * @return
     */
    void Overflow();

};

#endif /* OVERFLOWFILE_H_ */
