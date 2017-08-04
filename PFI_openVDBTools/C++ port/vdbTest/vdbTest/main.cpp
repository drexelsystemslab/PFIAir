//
//  main.cpp
//  vdbTest
//
//  Created by Hanjie Liu on 7/28/17.
//  Copyright © 2017 Hanjie Liu. All rights reserved.
//

#include <iostream>
#include "IOManager.hpp"

int main(int argc, const char * argv[]) {
    IOManager manager = IOManager();
    
    manager.import_stl("cube_new.stl");
    return 0;
}
