#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Signals.h>
#include <llvm/Support/FileSystem.h>
#include "cling/Utils/CrashHandler/CrashHandler.h"
#include "cling/Utils/CrashHandler/StackTraceCHAction.h"
#include "cling/Utils/CrashHandler/PrintDiagnosticCHAction.h"

using namespace llvm;

CrashHandler::CrashHandler() {}

void CrashHandler::execute() {
  for (unsigned int i=0; i<actions.size(); i++) {
    actions[i]->run(res);
  }
}

void CrashHandler::makeChain(std::vector<CHAction*> act) {
  actions = act;
}

void CrashHandler::dumpChain() {
  for (unsigned int i=0; i<actions.size(); i++) {
    llvm::errs() << i<< ": " << actions[i]->dump() << "\n";
  }
}

void llvm::handleCrashSignalWrapper(void*){
  llvm::CrashHandler CR;

  std::vector<CHAction*> actions;
  actions.push_back(new StackTraceCHAction(SmallString<128>("Print stacktrace to file")));
  actions.push_back(new PrintDiagnosticCHAction(SmallString<128>("Print diagnostic information")));

  CR.makeChain(actions);
  CR.execute();
  //CR.dumpChain(); DEBUG
  exit(1);
}