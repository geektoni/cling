#include <llvm/Support/raw_ostream.h>
#include "cling/Utils/CrashHandler/PrintDiagnosticCHAction.h"

void PrintDiagnosticCHAction::run(CHResult * res) {
  llvm::errs() << "A FATAL ERROR HAS OCCURRED\n"
          << "Please, send this file below to http://somesite.com to report the error:\n";
  for (unsigned int i=0; i < res->size(); i++) {
    llvm::errs() << (*res)[i].c_str() << "\n";
  }
}