//
//  IOManager.hpp
//  vdbTest
//
//  Created by Hanjie Liu on 7/28/17.
//  Copyright © 2017 Hanjie Liu. All rights reserved.
//

#ifndef IOManager_hpp
#define IOManager_hpp

#include <stdio.h>
#include <string>
#include <openvdb/openvdb.h>


class IOManager {
private:
    std::string _filename;
    
public:
    IOManager();
    
    openvdb::GridBase::Ptr import_model(const std::string file_name);
    bool import_stl(const std::string file_name);
};

#endif /* IOManager_hpp */
