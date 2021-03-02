//=============================================================================
// FILE:
//    HelloPass.cpp
//    https://github.com/banach-space/llvm-tutor/blob/main/HelloWorld/HelloWorld.cpp
//
// DESCRIPTION:
//    Visits all functions in a module, prints their names and the number of
//    arguments via stderr. Strictly speaking, this is an analysis pass (i.e.
//    the functions are not modified). However, in order to keep things simple
//    there's no 'print' method here (every analysis pass should implement it).
//
// USAGE:
//    1. Legacy PM
//      opt -load libHelloWorld.dylib -legacy-hello -disable-output `\`
//        <input-llvm-file>
//    2. New PM
//      opt -load-pass-plugin=libHelloWorld.dylib -passes="hello" `\`
//        -disable-output <input-llvm-file>
//
//
// License: MIT
//=============================================================================
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Transforms/IPO/PassManagerBuilder.h" // RegisterStandardPasses
#include "llvm/Support/CommandLine.h" // comand line arguments

using namespace llvm;

static cl::opt<bool> ClTestArgument(
    "hello-test",
    cl::desc("If set will print extra message"),
    cl::init(false));

//-----------------------------------------------------------------------------
// HelloPass implementation
//-----------------------------------------------------------------------------
// No need to expose the internals of the pass to the outside world - keep
// everything in an anonymous namespace.
namespace {

// This method implements what the pass does
void visitor(Function &F) {
    errs() << "Hello from: "<< F.getName() << "\n";
    errs() << "  number of arguments: " << F.arg_size() << "\n";
    if (ClTestArgument)
      errs() << "ClTestArgument is enabled!\n";
}

// New PM implementation
struct HelloPass : PassInfoMixin<HelloPass> {
  // Main entry point, takes IR unit to run the pass on (&F) and the
  // corresponding pass manager (to be queried if need be)
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    errs() << "I'm a function!\n";
    visitor(F);
    return PreservedAnalyses::all();
  }
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {
    errs() << "I'm a module!\n";
    return PreservedAnalyses::all();
  }
};

// Legacy PM implementation
struct LegacyHelloPass : public FunctionPass {
  static char ID;
  LegacyHelloPass() : FunctionPass(ID) {}
  // Main entry point - the name conveys what unit of IR this is to be run on.
  bool runOnFunction(Function &F) override {
    errs() << "Legacy\n";
    visitor(F);
    // Doesn't modify the input unit of IR, hence 'false'
    return false;
  }
};
} // namespace

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getHelloPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "Hello", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerFullLinkTimeOptimizationEarlyEPCallback(
                [](ModulePassManager &PM, OptimizationLevel Level) {
                PM.addPass(HelloPass());
                });
            /* runs at link time with thinLTO */
            PB.registerVectorizerStartEPCallback(
                [](FunctionPassManager &PM, OptimizationLevel Level) {
                PM.addPass(HelloPass());
                });
            PB.registerPipelineParsingCallback(
                [](StringRef Name, llvm::FunctionPassManager &PM,
                   ArrayRef<llvm::PassBuilder::PipelineElement>) {
                  if (Name == "hello-new-pm") {
                    PM.addPass(HelloPass());
                    return true;
                  }
                  return false;
                });
          }};
}

// This is the core interface for pass plugins. It guarantees that 'opt' will
// be able to recognize HelloPass when added to the pass pipeline on the
// command line, i.e. via '-passes=hello'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getHelloPassPluginInfo();
}

//-----------------------------------------------------------------------------
// Legacy PM Registration
//-----------------------------------------------------------------------------
// The address of this variable is used to uniquely identify the pass. The
// actual value doesn't matter.
char LegacyHelloPass::ID = 0;

// This is the core interface for pass plugins. It guarantees that 'opt' will
// recognize LegacyHelloPass when added to the pass pipeline on the command
// line, i.e.  via '--legacy-hello'
static RegisterPass<LegacyHelloPass>
    X("legacy-hello", "Hello Pass",
      true, // This pass doesn't modify the CFG => true
      false // This pass is not a pure analysis pass => false
    );

// static RegisterStandardPasses Y(
//     PassManagerBuilder::EP_OptimizerLast,
//     [](const PassManagerBuilder &Builder,
//       legacy::PassManagerBase &PM) { PM.add(new LegacyHelloPass()); });

static llvm::RegisterStandardPasses RegisterHelloLTOThinPass(
    llvm::PassManagerBuilder::EP_ModuleOptimizerEarly,
    [](const llvm::PassManagerBuilder &Builder,
       llvm::legacy::PassManagerBase &PM) { PM.add(new LegacyHelloPass()); });

static llvm::RegisterStandardPasses RegisterHelloLTOPass(
    llvm::PassManagerBuilder::EP_FullLinkTimeOptimizationEarly,
    [](const llvm::PassManagerBuilder &Builder,
       llvm::legacy::PassManagerBase &PM) { PM.add(new LegacyHelloPass()); });
