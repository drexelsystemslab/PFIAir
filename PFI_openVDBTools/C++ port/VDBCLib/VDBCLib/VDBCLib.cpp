/*
 *  VDBCLib.cpp
 *  VDBCLib
 *
 *  Created by Hanjie Liu on 7/24/17.
 *  Copyright © 2017 Hanjie Liu. All rights reserved.
 *
 */

#include <iostream>
#include "VDBCLib.hpp"
#include "VDBCLibPriv.hpp"

void VDBCLib::HelloWorld(const char * s)
{
    VDBCLibPriv *theObj = new VDBCLibPriv;
    theObj->HelloWorldPriv(s);
    delete theObj;
};

void VDBCLibPriv::HelloWorldPriv(const char * s) 
{
    std::cout << s << std::endl;
};

