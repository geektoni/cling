#ifndef CLING_CHACTION_H
#define CLING_CHACTION_H

#include <llvm/ADT/SmallString.h>
typedef std::vector<llvm::SmallString<128>> CHResult;

class CHAction {

public:
    virtual void run(CHResult * res);
    virtual llvm::SmallString<128> dump() {return description;}

protected:
    llvm::SmallString<128> description;

};


#endif //CLING_CHACTION_H
