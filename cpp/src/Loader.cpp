#include <boost/unordered_map.hpp>
#include "Loader.hpp"

static boost::unordered_map<fs::path,CompilationUnit*> previouslySeenUnits;

static CompilationUnit* privateLoad(fs::path p, int stage) {
    auto unit = previouslySeenUnits.find(p);
    
    if (unit == previouslySeenUnits.end()) {
        return new CompilationUnit(p.string(), stage);
    } else {
        return (*unit).second;
    }
}

CompilationUnit* Loader::Load(std::string path, int stage) {
    fs::path newPath(path);
    
    if (!exists(newPath)) {
        std::cerr << "Tried to load directory/file " << newPath.string() << " but it was not found" << std::endl;
        return nullptr;
    }
    
    return privateLoad(path,stage);
}

CompilationUnit* Loader::Load(std::string currentPath, std::string relativePath) {
    
    
    /* This will need re-working if we want to load from a URL etc, but for now this will suffice
     
     Suppose we are working on "directory1/path" and we import "../directory2/path" concatentation is sufficient.
     If its an absolute path we can simply use the initial 
     
    */
    fs::path newPath(currentPath + std::string("/") + relativePath);
    
    if (!exists(newPath)) {
        std::cerr << "Tried to load directory/file " << newPath.string() << " but it was not found" << std::endl;
        return nullptr;
    }
    
    return privateLoad(newPath, INT_MAX);
}

