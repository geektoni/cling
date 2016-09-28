#ifndef CLING_CRASHREPORTER_H
#define CLING_CRASHREPORTER_H

#include <system_error>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/SmallString.h>
#include "CHAction.h"

namespace llvm {

    class CrashHandler {
    private:
        std::vector<CHAction*> actions;
        CHResult * res = new CHResult;

    public:
        CrashHandler();

        void execute();
        void makeChain(std::vector<CHAction*> actions);
        void dumpChain();

    };
    void handleCrashSignalWrapper(void*);
}


#endif //CLING_CRASHREPORTER_H
