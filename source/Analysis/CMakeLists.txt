ops_filesin(INTERFACE "../../include/Analysis/"
  AbstractDepGraph.h
  CalculationGraph/CalculationGraph.h
  CalculationGraph/CalculationGraphBase.h
  CalculationGraph/CalculationGraphBuilder.h
  CalculationGraph/ExpressionGraph.h
  Clones/HashDeepWalker.h
  Clones/CloneFinder.h
  CallGraph.h
  ComplexOccurrenceAnalysis/GrouppedOccurrences.h
  ConsistCheck/Conditions.h
  ConsistCheck/IConsistCheckService.h
  ControlFlowGraph.h
  ControlFlowGraphEx.h
  DeadDeclarations.h
  DepGraph/DepGraph.h
  DepGraph/id.h
  DepGraph/LoopStruct.h
  DepGraph/Status.h
  DepGraph/SubProcAnalysis.h
  DepGraph/DepGraphEx/DepGraphEx.h
  DepGraph/DepGraphEx/LoopHelpers.h
  DepGraph/DepGraphEx/Predicates.h
  DepGraph/DepGraphEx/Simplifier.h
  DominanceFrontierGraph.h
  ExpressionOrder.h
  Frames/FrameDataSpecification.h
  Frames/Frame.h
  Frames/FramesGraph.h
  Frames/FramesPartitioner.h
  Frames/IFramesPartitioner.h
  Frames/IProfiler.h
  Frames/ProfilerBridge.h
  Frames/BlockNestAnalizer.h
  LatticeGraph/ComplexCondition.h
  LatticeGraph/Correspondence.h
  LatticeGraph/ElemLatticeGraph.h
  LatticeGraph/ExtendedQuast.h
  LatticeGraph/GaussianElimination.h
  LatticeGraph/LatticeGraph.h
  LatticeGraph/LinDataForLatticeGraph.h
  LatticeGraph/LinearLib.h
  LatticeGraph/LoopIndex.h
  LatticeGraph/PIP.h
  LatticeGraph/ParamPoint.h
  LatticeGraph/RationalNumber.h
  LatticeGraph/voevodin.h
  LoopsInterchange/LoopsInterchangeAnalysis.h
  MinControlEdgeSet.h
  Montego/AliasAnalysis/AliasInterface.h
  Montego/AliasAnalysis/AliasImplementation.h
  Montego/AliasAnalysis/AliasTypes.h
  Montego/AliasAnalysis/CanonicalForm.h
  Montego/AliasAnalysis/SetAbstractMemoryCell2.h
  Montego/AliasAnalysis/MemoryCell.h
  Montego/DependenceGraph/DependenceGraph.h
  Montego/DependenceGraph/DependenceGraphArc.h
  Montego/DependenceGraph/DependenceGraphVertex.h
  Montego/DependenceGraph/DependenceLevel.h
  Montego/DependenceGraph/IndexAnalysis.h
  Montego/DependenceGraph/SymbolicAnalysis.h
  Montego/Loops.h
  Montego/Occurrence.h
  Montego/OccurrenceCoeffs.h
  Montego/OccurrenceContainer.h
  Montego/SafeStatus.h
  ParallelLoops.h
  Profiler/Profiler.h
  Profiler/ProfTextBuilder.h
  Renewal/AliasAnalysis/IAliasAnalysisService.h
  Renewal/Occurrence/OccurrenceHelpers.h
  Renewal/Occurrence/Occurrences.h
  Renewal/OccurrenceAnalysis/IOccurrenceAnalysisService.h
  SSAForm/SSAForm.h
  SSAForm/VirtualRegistry.h
  Statistics.h
  SymbolPredicateFinder.h
  InductionVariables/InductionVariables.h
)

ops_project(
    Analysis LIB
  SOURCES
  ${INTERFACE}
  AnalysisPlugin.h
  AnalysisPlugin.cpp
  AbstractDepGraph.cpp
  CalculationGraph/CalculationGraph.cpp
  CalculationGraph/CalculationGraphBase.cpp
  CalculationGraph/CalculationGraphBuilder.cpp
  Clones/HashDeepWalker.cpp
  Clones/CloneFinder.cpp
  CallGraph/CallGraph.cpp
  ComplexOccurrenceAnalysis/GrouppedOccurrences.cpp
  ConsistCheck/Conditions.cpp
  ConsistCheck/ConsistCheckWalker.h
  ConsistCheck/ConsistCheckWalker.cpp
  ConsistCheck/ConsistCheckService.h
  ConsistCheck/ConsistCheckService.cpp
  ControlFlowGraph/ControlFlowGraph.cpp
  ControlFlowGraphEx/ControlFlowGraphEx.cpp
  DeadDeclarations.cpp
  DepGraph/id.cpp
  DepGraph/src/DepGraph.cpp
  DepGraph/src/LoopStruct.cpp
  DepGraph/src/SubProcAnalysis.cpp
  DepGraph/src/DepGraphEx/DepGraphEx.cpp
  DepGraph/src/DepGraphEx/LoopHelpers.cpp
  DepGraph/src/DepGraphEx/Predicates.cpp
  DepGraph/src/DepGraphEx/Simplifier.cpp
  DominanceFrontierGraph/DominanceFrontierGraph.cpp
  ExpressionOrder.cpp
  Frames/FrameDataSpecification.cpp
  Frames/FramesGraph.cpp
  Frames/FramesPartitioner.cpp
  Frames/ProfilerBridge.cpp
  Frames/ProfilerFramesPartitioningWalker.h
  Frames/BlockNestAnalizer.cpp
  LatticeGraph/ComplexCondition.cpp
  LatticeGraph/Correspondence.cpp
  LatticeGraph/ElemLatticeGraph.cpp
  LatticeGraph/ExtendedQuast.cpp
  LatticeGraph/GaussianElimination.cpp
  LatticeGraph/LatticeGraph.cpp
  LatticeGraph/LinearLib.cpp
  LatticeGraph/LoopIndex.cpp
  LatticeGraph/PIP.cpp
  LatticeGraph/ParamPoint.cpp
  LatticeGraph/RationalNumber.cpp
  LatticeGraph/voevodin.cpp
  LatticeGraph/CountIntPoints/FunctionsVerge.cpp
  LatticeGraph/CountIntPoints/FunctionsVerge.h
  LatticeGraph/CountIntPoints/IOfunctions.cpp
  LatticeGraph/CountIntPoints/IOfunctions.h
  LatticeGraph/CountIntPoints/StructuresVerge.h
  LatticeGraph/CountIntPoints/LLL.h
  LatticeGraph/CountIntPoints/LLL.cpp
  LatticeGraph/CountIntPoints/RationalMatrix.cpp
  LatticeGraph/CountIntPoints/RationalMatrix.h
  LoopsInterchange/LoopsInterchangeAnalysis.cpp
  MinControlEdgeSet/MinControlEdgeSet.cpp
  Montego/AliasAnalysis/ExpressionAnalyser.h
  Montego/AliasAnalysis/ExpressionAnalyser.cpp
  Montego/AliasAnalysis/DynamicMemoryAllotmentSearchVisitor.h
  Montego/AliasAnalysis/DynamicMemoryAllotmentSearchVisitor.cpp
  Montego/AliasAnalysis/AliasProcedureAnalyzer.h
  Montego/AliasAnalysis/AliasProcedureAnalyzer.cpp
  Montego/AliasAnalysis/SetAbstractMemoryCell2.cpp
  Montego/AliasAnalysis/MemoryCell.cpp
  Montego/AliasAnalysis/ProgramState.cpp
  Montego/AliasAnalysis/MemoryCellContainer.h
  Montego/AliasAnalysis/MemoryCellContainer.cpp
  Montego/AliasAnalysis/FunctionContext.h
  Montego/AliasAnalysis/FunctionContext.cpp
  Montego/AliasAnalysis/AliasInterface.cpp
  Montego/AliasAnalysis/CanonicalForm.cpp
  Montego/AliasAnalysis/ProgramState.h
  Montego/AliasAnalysis/AliasTypes.cpp
  Montego/AliasAnalysis/AliasAnalysisContext.h
  Montego/AliasAnalysis/AliasAnalysisContext.cpp
  Montego/AliasAnalysis/Navigation.h
  Montego/AliasAnalysis/Navigation.cpp
  Montego/DependenceGraph/DependenceGraph.cpp
  Montego/DependenceGraph/DependenceGraphArc.cpp
  Montego/DependenceGraph/DependenceGraphVertex.cpp
  Montego/DependenceGraph/DependenceLevel.cpp
  Montego/DependenceGraph/IndexAnalysis.cpp
  Montego/DependenceGraph/SymbolicAnalysis.cpp
  Montego/Loops.cpp
  Montego/SafeStatus.cpp
  Montego/Occurrence.cpp
  Montego/OccurrenceCoeffs.cpp
  Montego/OccurrenceContainer.cpp
  Montego/OccurrenceContainerBuilder.h
  Montego/OccurrenceContainerBuilder.cpp
  Montego/OccurSearchVisitor.h
  Montego/OccurSearchVisitor.cpp
  ParallelLoops/ParallelLoops.cpp
  ParallelLoops/Analysers.cpp
  ParallelLoops/Privatization.cpp
  Profiler/Profiler.cpp
  Profiler/ProfilingInterprocedural.h
  Profiler/ProfilingInterprocedural.cpp
  Profiler/ProfilingWithinSubroutine.h
  Profiler/ProfilingWithinSubroutine.cpp
  Profiler/ProfTextBuilder.cpp
  Profiler/UsingPlatformSettings.h
  Profiler/UsingPlatformSettings.cpp
  Renewal/AliasAnalysis/AliasAnalysisService.h
  Renewal/AliasAnalysis/AliasAnalysisService.cpp
  Renewal/AliasAnalysis/AliasAnalysisServiceState.h
  Renewal/AliasAnalysis/IOccurrenceAnalyzerHolder.h
  Renewal/Occurrence/OccurrenceHelpers.cpp
  Renewal/Occurrence/Occurrences.cpp
  Renewal/OccurrenceAnalysis/OccurrenceAnalysisService.h
  Renewal/OccurrenceAnalysis/OccurrenceAnalysisService.cpp
  Renewal/OccurrenceAnalysis/OccurrenceAnalysisServiceState.h
  SSAForm/SSAForm.cpp
  SSAForm/VirtualRegistry.cpp
  Statistics.cpp
  SymbolPredicateFinder/SymbolPredicateFinder.cpp
  InductionVariables/InductionVariables.cpp
  LIBRARIES
    OPS_Core
    Shared
    OPS_Stage
    FrontTransforms
  PUBLIC
)

add_subdirectory(DominanceFrontierGraph/testmod)
add_subdirectory(InductionVariables/testmod)
add_subdirectory(LatticeGraph)
add_subdirectory(Montego/AliasAnalysis/testmod)
add_subdirectory(Montego/AliasAnalysis/perftest)
add_subdirectory(Profiler)
add_subdirectory(SSAForm/testmod)
