#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include <vector>

using namespace llvm;

namespace {

struct SkeletonPass : public PassInfoMixin<SkeletonPass> {
    
    AllocaInst* findStoreVal(CallInst* CI) {
        std::vector<Instruction*> workList;
        workList.push_back(CI);

        while(!workList.empty()) {
            Instruction* work = workList.back();
            workList.pop_back();

            if (StoreInst* storeInst = dyn_cast<StoreInst>(work)) {
                // The store destination must be a stack variable
                if (AllocaInst* allocaInst = dyn_cast<AllocaInst>(storeInst->getPointerOperand())) {
                    return allocaInst;                    
                } 
            } else {
                // Find all users
                for (User* user: work->users()) {
                    if (Instruction* I = dyn_cast<Instruction>(user)) {
                        workList.push_back(I);
                    }
                }
            }
        }
        return nullptr;
    }

    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
        FunctionType* slowMallocFnType = M.getFunction("malloc")->getFunctionType();
        Function* slowMallocFn = Function::Create(slowMallocFnType, Function::ExternalLinkage, "slow_malloc", M);
        for (auto &F : M) {
//            errs() << "I saw a function called " << F.getName() << "!\n";
            for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
                // Track all call instructions
                if (CallInst* CI = dyn_cast<CallInst>(&*I)) {
                    bool isMalloc = false;
                    if (Function* function = CI->getCalledFunction()) {
                        if (function->getName() == "malloc") {
                            /*
                            errs() << "Saw call to malloc: \n";
                            I->dump();
                            */
                            isMalloc = true;
                        }
                    }
                    if (isMalloc) {
                        AllocaInst* storeDestVal = findStoreVal(CI);
                        if (storeDestVal) {
                            // Count how many users of storeDestVal exist
                            // silly code, but it works. Better API exists, but we will not complicate it
                            int count = 0;
                            for (User* user: storeDestVal->users()) {
                                ++count;
                            }
                            errs() << "Heap object accessed " << count << " times\n";                            
                            if (count < 3) { // 1 access is for storing the heap object
                                CI->setCalledFunction(slowMallocFn);
                                errs() << "Replaced \n";
                                CI->dump();
                            }
                        }
                    }
                }
            }
        }
        return PreservedAnalyses::all();
    };
};

}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "Skeleton pass",
        .PluginVersion = "v0.1",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                    MPM.addPass(SkeletonPass());
                });
        }
    };
}
