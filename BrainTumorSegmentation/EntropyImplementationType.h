#pragma once

// Was declared in DecisionTreeTrainer but caused circular reference in ForestParameters.cpp
enum enumComputeEntropyImplementationType { ENTROPY_TYPE_OriginalImplementation = 0, 
        ENTROPY_TYPE_MultiplyClassPriorsBeforeEntropy = 1, 
        ENTROPY_TYPE_MultiplyClassPriorsAfterEntropy = 2,
        ENTROPY_TYPE_MultiplyClassPriorsAfterEntropyNoDirichlet = 3 };

static const char* arrstrEnumComputeEntropyImplementationType[] = {"OriginalImplementation", 
        "MultiplyClassPriorsBeforeEntropy", 
        "MultiplyClassPriorsAfterEntropy",
        "MultiplyClassPriorsAfterEntropyNoDirichlet"};

