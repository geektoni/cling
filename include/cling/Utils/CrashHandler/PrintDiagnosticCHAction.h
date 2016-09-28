#ifndef CLING_PRINTDIAGNOSTICCHACTION_H
#define CLING_PRINTDIAGNOSTICCHACTION_H

#include "CHAction.h"

class PrintDiagnosticCHAction : public CHAction {

public:
    PrintDiagnosticCHAction(llvm::SmallString<128> description) {this->description = description;}
    virtual void run(CHResult * res);
};


#endif //CLING_PRINTDIAGNOSTICCHACTION_H
