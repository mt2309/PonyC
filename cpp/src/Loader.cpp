//
//  Loader.cpp
//  ponyC
//
//  Created by Michael Thorpe on 05/02/2012.
//
//

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wweak-vtables"
#pragma GCC diagnostic ignored "-Wpadded"
#pragma GCC diagnostic ignored "-Wdisabled-macro-expansion"
#pragma GCC diagnostic ignored "-Wmissing-noreturn"
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#include <boost/unordered_map.hpp>
#include <boost/filesystem.hpp>
#pragma GCC diagnostic pop

#include "Loader.hpp"

namespace fs = boost::filesystem;

static boost::unordered_map<fs::path,CompilationUnit*> *previouslySeenUnits;

static CompilationUnit* privateLoad(fs::path p, int stage) {
    auto unit = previouslySeenUnits->find(p);
    
    if (unit == previouslySeenUnits->end()) {
        return new CompilationUnit(p.string(), stage);
    } else {
        return (*unit).second;
    }
}

static fs::path createPath(std::string currentPath, std::string relativePath) {
    
    if (relativePath.length() > 0) {
        if (relativePath.at(0) != '/') {
            return fs::path(currentPath + std::string("/") + relativePath);
        } else {
            return fs::path(relativePath);
        }
    }
    
    // Return an empty path (perhaps).
    fs::path p;
    
    return p;
}

CompilationUnit* Loader::Load(std::string path, int stage) {
    
    if (previouslySeenUnits == nullptr) {
        previouslySeenUnits = new boost::unordered_map<fs::path, CompilationUnit*>();
    }
    
    fs::path newPath(path);
    
    if (!exists(newPath)) {
        std::cerr << "Tried to load directory/file " << newPath.string() << " but it was not found" << std::endl;
        return nullptr;
    }
    
    return privateLoad(path,stage);
}

CompilationUnit* Loader::Load(std::string currentPath, std::string relativePath) {
    
    if (previouslySeenUnits == nullptr) {
        previouslySeenUnits = new boost::unordered_map<fs::path, CompilationUnit*>();
    }
    
    /* This will need re-working if we want to load from a URL etc, but for now this will suffice
     
     Suppose we are working on "directory1/path" and we import "../directory2/path" concatentation is sufficient.
     If its an absolute path we can simply use the initial 
     
    */
    fs::path newPath = createPath(currentPath, relativePath);
    
        
    if (!exists(newPath)) {
        std::cerr << "Tried to load directory/file " << newPath.string() << " but it was not found" << std::endl;
        return nullptr;
    }
    
    return privateLoad(newPath, INT_MAX);
}

