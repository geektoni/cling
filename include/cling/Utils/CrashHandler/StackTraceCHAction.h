#ifndef CLING_STACKTRACECHACTION_H
#define CLING_STACKTRACECHACTION_H

#include "CHAction.h"

class StackTraceCHAction : public CHAction  {

public:
    StackTraceCHAction(llvm::SmallString<128> description) {this->description = description;}
    virtual void run(CHResult * res);

};

#endif //CLING_STACKTRACECHACTION_H
