# How to use LLVM passes

## How to run

```
mkdir build
cd build
cmake ..
make
cd ..
# lit should also work
llvm-lit build/test
```

This README is mostly just a braindump that might or might not be useful for anybody.
No guarantees for correct information ;)

## Legacy vs New Pass Manager

If in doubt, save yourself some trouble and use the old pass manager.

With the new pass manager until recently callbacks for LTO were missing
and running passes with `lld` with the new pass manager is not yet working.

## Possible Entry Points

### Legacy Pass Manager

Extract from `llvm/include/llvm/Transforms/IPO/PassManagerBuilder.h`:

| Entry Point                      | Description                                                                           |
| -------------------------------- | ------------------------------------------------------------------------------------- |
| EP_EarlyAsPossible               | allows adding passes before any other transformations                                 |
| EP_ModuleOptimizerEarly          | allows adding passes just before the main module-level optimization passes            |
| EP_OptimizerLast                 | allows adding passes that run after everything else                                   |
| EP_VectorizerStart               | allows adding optimization passes before the vectorizer and other highly target specific optimization passes are executed        |
| EP_EnabledOnOptLevel0            | allows adding passes that should not be disabled by O0 optimization level             |
| EP_FullLinkTimeOptimizationEarly | allows adding passes that run at Link Time, before Full Link Time Optimization        |
| EP_FullLinkTimeOptimizationLast  | allows adding passes that run at Link Time, after Full Link Time Optimization         |

### New Pass Manager

Extract from `llvm/include/llvm/Passes/PassBuilder.h`:

| Entry Point                       | Description                                                                                                        |
| --------------------------------- | ------------------------------------------------------------------------------------------------------------------ |
| registerPipelineParsingCallback   | for `opt`                                                                                                          |
| registerOptimizerLastEPCallback   | adding optimizations at the very end of the function optimization pipeline                                         |
| registerPipelineStartEPCallback   | adding optimization once at the start of the pipeline                                                              |
| registerVectorizerStartEPCallback | adding optimization passes before the vectorizer and other highly target specific optimization passes are executed |

## Pipelines

There are several pipelines, the chosen pipeline depends on e.g. if LTO is used or the optimization level.

### Legacy Pass Manager

Defined in `llvm/lib/Transforms/IPO/PassManagerBuilder.cpp`:

| Optimizaton Level 0   | Default                  | thinLTO                  | LTO                              |
| --------------------- | ------------------------ | ------------------------ | -------------------------------- |
| EP_EnabledOnOptLevel0 | EP_ModuleOptimizerEarly  | EP_ModuleOptimizerEarly  | EP_FullLinkTimeOptimizationEarly |
|                       | EP_CGSCCOptimizerLate    | EP_CGSCCOptimizerLate    | EP_FullLinkTimeOptimizationLast  |
|                       | EP_LateLoopOptimizations | EP_LateLoopOptimizations |                                  |
|                       | EP_LoopOptimizerEnd      | EP_LoopOptimizerEnd      |                                  |
|                       | EP_ScalarOptimizerLate   | EP_ScalarOptimizerLate   |                                  |
|                       | EP_VectorizerStart       | EP_VectorizerStart       |                                  |
|                       | EP_OptimizerLast         | EP_OptimizerLast         |                                  |

### New Pass Manager

Defined in `llvm/lib/Passes/PassBuilderPipelines.cpp`:

#### External

without LTO:

| Optimizaton Level 0 (buildO0DefaultPipeline) | Default (buildPerModuleDefaultPipeline)               |
| -------------------------------------- | ----------------------------------------------------------- |
| PipelineStartEPCallbacks               | PipelineStartEPCallbacks                                    |
| PipelineEarlySimplificationEPCallbacks | PipelineEarlySimplificationEPCallbacks                      |
| CGSCCOptimizerLateEPCallbacks          | CGSCCOptimizerLateEPCallbacks (if `EnableModuleInliner`)    |
| LateLoopOptimizationsEPCallbacks       | LateLoopOptimizationsEPCallbacks                            |
| LoopOptimizerEndEPCallbacks            | LoopOptimizerEndEPCallbacks                                 |
| ScalarOptimizerLateEPCallbacks         | ScalarOptimizerLateEPCallbacks                              |
| VectorizerStartEPCallbacks             | VectorizerStartEPCallbacks                                  |
| OptimizerLastEPCallbacks               | OptimizerLastEPCallbacks                                    |

with LTO (unsure right now):

| buildThinLTOPreLinkDefaultPipeline (compile time) | buildThinLTODefaultPipeline (link time) | buildLTOPreLinkDefaultPipeline (compile time) | buildLTODefaultPipeline (link time) |
| -------------------------------------- | --------------------------------------- | -------------------------------------- | - |
| PipelineStartEPCallbacks               |                                         | PipelineStartEPCallbacks               | FullLinkTimeOptimizationEarlyEPCallbacks (added recently) |
| PipelineEarlySimplificationEPCallbacks | PipelineEarlySimplificationEPCallbacks  | PipelineEarlySimplificationEPCallbacks | FullLinkTimeOptimizationLastEPCallbacks (added recently) |
| CGSCCOptimizerLateEPCallbacks          | CGSCCOptimizerLateEPCallbacks           | CGSCCOptimizerLateEPCallbacks          | |
| LateLoopOptimizationsEPCallbacks       | LateLoopOptimizationsEPCallbacks        | LateLoopOptimizationsEPCallbacks       | |
| LoopOptimizerEndEPCallbacks            | LoopOptimizerEndEPCallbacks             | LoopOptimizerEndEPCallbacks            | |
| ScalarOptimizerLateEPCallbacks         | ScalarOptimizerLateEPCallbacks          | ScalarOptimizerLateEPCallbacks         | |
|                                        | VectorizerStartEPCallbacks              | VectorizerStartEPCallbacks             | |
| OptimizerLastEPCallbacks               | OptimizerLastEPCallbacks                | OptimizerLastEPCallbacks               | |


#### Internal (used by other pipelines)

`buildO1FunctionSimplificationPipeline`:
* `LateLoopOptimizationsEPCallbacks`
* `LoopOptimizerEndEPCallbacks`
* `ScalarOptimizerLateEPCallbacks`

`buildFunctionSimplificationPipeline`:
* call `buildO1FunctionSimplificationPipeline` (only with `-O1`)
* `LateLoopOptimizationsEPCallbacks`
* `LoopOptimizerEndEPCallbacks`
* `ScalarOptimizerLateEPCallbacks`

`buildInlinerPipeline`:
* `CGSCCOptimizerLateEPCallbacks`
* call `buildFunctionSimplificationPipeline`

`buildModuleInlinerPipeline`:
* call `buildFunctionSimplificationPipeline`

`buildModuleSimplificationPipeline`:
* `PipelineEarlySimplificationEPCallbacks`
* call `buildModuleInlinerPipeline` if `EnableModuleInliner` else `buildInlinerPipeline`

`buildModuleOptimizationPipeline`:
* `VectorizerStartEPCallbacks`
* `OptimizerLastEPCallbacks`

## Legacy Pass Manager Callbacks

Loading Passes:
RegisterPass (used with `opt`)
- Pass goes into PassRegistry
- through listener the cli option is registered (in `passRegistered`)

RegisterStandardPasses (used with `clang`/`lld`?)
calls `addGlobalExtension`
`addExtensionsToPM` adds `GlobalExtension` passes for the correct extension point

Running Passes:
to enable use (`--legacy-hello`):
loading the pass enabled that cli option:
setting it will trigger `handleOccurrence` (in `Commandline.h`) which sets pass as a command line option
within `opt` this is then located in `PassList`

## New Pass Manager Callbacks

Loading Passes:
`-load-pass-plugin` loads passes into `PassPlugins` (with `opt`)
`PassPlugin->registerPassBuilderCallbacks(PB);` in `runPassPipeline` registers callbacks to register the pass

`fpass-plugin` loads pass (with `clang`) directly into `PassPlugins`
in `EmitAssemblyWithNewPassManager` the `PassPlugins` get loaded and `registerPassBuilderCallbacks` is called

Running Passes:
* registerPipelineParsingCallback (used with `opt`) (doesn't run on clang)
`PassPipeline` is passed in through `-passes=""`
runs within `parsePassPipeline` (called by `runPassPipeline`) (no control where in the pipeline the pass is inserted?)

## LTO: Full vs Thin
```
clang -fuse-ld=lld -flto file1.c file2.c -o a.out
clang -fuse-ld=lld -flto=thin file1.c file2.c -o a.out
```

With full LTO you have visiblity over the entire linked blob (both `file1.c` and `file2.c`) and apply passes single-threaded.
With thin LTO you run passes in parallel on `file1.c` and `file2.c` and apply passes on both multi-threaded.

## How to apply passes

Applying passes with `opt` will just do that, you register them in the pipeline and specify the name of passes to run.
Applying passes with `clang` or `lld` (with LTO) requires you to set the extension point on where the pass should run.

With the legacy pass manager with `clang` and `lld` passes get run automatically without an additional command line argument. With `opt` you need to supply a commmand line argument to run the pass.

Applying passes is different for pretty much all the three possible options (`clang`, `opt`, `lld`) and also differs between the new and old pass manager.
To make this a bit more clear, this repo should provide some examples on how to do it.

## Miscellaneous notes

If you use the new pass manager with `lld` you need to also include `-mllvm=-load=` to load any additional CLI arguments your pass is exposing.
(It's not working anyways currently but hopefully soon if https://reviews.llvm.org/D120490 is merged).

New PM extension points for the LTO pipeline also only got merged very recently (https://github.com/llvm/llvm-project/commit/942efa5927ae7514d8e03d90e67284945b7c40fe)

The code in this repo was tested on llvm-project (`374bb6dd8090ffaca5aad86b2d1cd9acad401f3a`).

the `-fpass-plugin` option of `clang` is only to run compile time passes (even with LTO).

the `--lto-newpm-passes` option of `lld` will only run the specific passes, not any default pipeline (AFAIK).
