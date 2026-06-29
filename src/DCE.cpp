// Dead code elimination
// This removes globally unused instructions, and locally killed instructions
// Where 'local' means local to a basic block

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <vector>

using namespace llvm;

namespace {
  // This method implements what the pass does
  void visitor(Function &F) {
	std::vector<Instruction *> InstrsForDeletion;
	
    for (auto &BB : F) {
      for (auto &CandidateI : BB) {
        if (CandidateI.use_empty() && !CandidateI.mayHaveSideEffects() && !CandidateI.isTerminator()) {
		  InstrsForDeletion.push_back(&CandidateI);
		}
      }
    }

    // Delete the instructions
    for (auto *I : InstrsForDeletion) {
	  I->eraseFromParent();
    }
  }

// New PM implementation
struct MyDCE : PassInfoMixin<MyDCE> {
  // Main entry point, takes IR unit to run the pass on (&F) and the
  // corresponding pass manager (to be queried if need be)
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    visitor(F);
    return PreservedAnalyses::none();
  }

  // Without isRequired returning true, this pass will be skipped for functions
  // decorated with the optnone LLVM attribute. Note that clang -O0 decorates
  // all functions with optnone.
  static bool isRequired() { return true; }
};
} // namespace

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getMyDCEPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "MyDCE", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "my-dce") {
                    FPM.addPass(MyDCE());
                    return true;
                  }
                  return false;
                });
          }};
}

// This is the core interface for pass plugins. It guarantees that 'opt' will
// be able to recognize DCE when added to the pass pipeline on the
// command line, i.e. via '-passes=my-dce'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getMyDCEPluginInfo();
}


