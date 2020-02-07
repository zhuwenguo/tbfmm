#include "spacial/tbfmortonspaceindex.hpp"
#include "spacial/tbfspacialconfiguration.hpp"
#include "utils/tbfrandom.hpp"
#include "core/tbfcellscontainer.hpp"
#include "core/tbfparticlescontainer.hpp"
#include "core/tbfparticlesorter.hpp"
#include "core/tbftree.hpp"
#include "kernels/testkernel/tbftestkernel.hpp"
#include "algorithms/tbfalgorithmselecter.hpp"
#include "utils/tbftimer.hpp"
#include "kernels/counterkernels/tbfinteractioncounter.hpp"
#include "kernels/counterkernels/tbfinteractiontimer.hpp"
#include "algorithms/tbfalgorithmperiodictoptree.hpp"


#include <iostream>


int main(){
    using RealType = double;
    const int Dim = 3;

    /////////////////////////////////////////////////////////////////////////////////////////

    const std::array<RealType, Dim> BoxWidths{{1, 1, 1}};
    const long int TreeHeight = 5;
    const std::array<RealType, Dim> inBoxCenter{{0.5, 0.5, 0.5}};

    const TbfSpacialConfiguration<RealType, Dim> configuration(TreeHeight, BoxWidths, inBoxCenter);

    /////////////////////////////////////////////////////////////////////////////////////////

    const long int NbParticles = 100000;

    TbfRandom<RealType, Dim> randomGenerator(configuration.getBoxWidths());

    std::vector<std::array<RealType, Dim>> particlePositions(NbParticles);

    for(long int idxPart = 0 ; idxPart < NbParticles ; ++idxPart){
        particlePositions[idxPart] = randomGenerator.getNewItem();
    }

    /////////////////////////////////////////////////////////////////////////////////////////

    constexpr long int NbDataValuesPerParticle = Dim;
    constexpr long int NbRhsValuesPerParticle = 1;
    using MultipoleClass = std::array<long int,1>;
    using LocalClass = std::array<long int,1>;
    const long int inNbElementsPerBlock = 50;
    const bool inOneGroupPerParent = false;
    using SpacialSystemPeriodic = TbfDefaultSpaceIndexTypePeriodic<RealType>;
    const long int LastWorkingLevel = TbfDefaultLastLevelPeriodic;
    using TreeClass = TbfTree<RealType,
                              RealType,
                              NbDataValuesPerParticle,
                              long int,
                              NbRhsValuesPerParticle,
                              MultipoleClass,
                              LocalClass,
                              SpacialSystemPeriodic>;

    /////////////////////////////////////////////////////////////////////////////////////////

    TbfTimer timerBuildTree;

    TreeClass tree(configuration, inNbElementsPerBlock, particlePositions, inOneGroupPerParent);

    timerBuildTree.stop();
    std::cout << "Build the tree in " << timerBuildTree.getElapsed() << std::endl;

    for(long int idxExtraLevel = -1 ; idxExtraLevel < 5 ; ++idxExtraLevel){
        using AlgorithmClass = TbfAlgorithmSelecter::type<RealType, TbfTestKernel<RealType, SpacialSystemPeriodic>, SpacialSystemPeriodic>;
        using TopPeriodicAlgorithmClass = TbfAlgorithmPeriodicTopTree<RealType, TbfTestKernel<RealType, SpacialSystemPeriodic>, MultipoleClass, LocalClass, SpacialSystemPeriodic>;

        AlgorithmClass algorithm(configuration, LastWorkingLevel);
        TopPeriodicAlgorithmClass topAlgorithm(configuration, idxExtraLevel);

        TbfTimer timerExecute;

        // Bottom to top
        algorithm.execute(tree, TbfAlgorithmUtils::TbfBottomToTopStages);
        // Periodic at the top (could be done in parallel with TbfTransferStages)
        topAlgorithm.execute(tree);
        // Transfer (could be done in parallel with topAlgorithm.execute)
        algorithm.execute(tree, TbfAlgorithmUtils::TbfTransferStages);
        // Top to bottom
        algorithm.execute(tree, TbfAlgorithmUtils::TbfTopToBottomStages);

        timerExecute.stop();
        std::cout << "Execute in " << timerExecute.getElapsed() << std::endl;

        std::cout << "Perform the periodic FMM with parameters:" << std::endl;
        std::cout << " - idxExtraLevel: " << idxExtraLevel << std::endl;
        std::cout << " - Nb repeat per dim: " << topAlgorithm.getNbRepetitionsPerDim() << std::endl;
        std::cout << " - Number of times the real box is duplicated: " << topAlgorithm.getNbTotalRepetitions() << std::endl;
        std::cout << " - Repeatition interves: " << TbfUtils::ArrayPrinter(topAlgorithm.getRepetitionsIntervals().first)
                  << " " << TbfUtils::ArrayPrinter(topAlgorithm.getRepetitionsIntervals().second) << std::endl;
        std::cout << " - original configuration: " << configuration << std::endl;
        std::cout << " - top tree configuration: " << TopPeriodicAlgorithmClass::GenerateAboveTreeConfiguration(configuration,idxExtraLevel) << std::endl;
    }

    for(long int idxExtraLevel = -1 ; idxExtraLevel < 5 ; ++idxExtraLevel){ // Same as above but with interaction counter
        using KernelClass = TbfInteractionCounter<TbfTestKernel<RealType, SpacialSystemPeriodic>>;
        using AlgorithmClass = TbfAlgorithmSelecter::type<RealType, KernelClass, SpacialSystemPeriodic>;
        using TopPeriodicAlgorithmClass = TbfAlgorithmPeriodicTopTree<RealType, KernelClass, MultipoleClass, LocalClass, SpacialSystemPeriodic>;

        AlgorithmClass algorithm(configuration, LastWorkingLevel);
        TopPeriodicAlgorithmClass topAlgorithm(configuration, idxExtraLevel);

        TbfTimer timerExecute;

        // Bottom to top
        algorithm.execute(tree, TbfAlgorithmUtils::TbfBottomToTopStages);
        // Periodic at the top (could be done in parallel with TbfTransferStages)
        topAlgorithm.execute(tree);
        // Transfer (could be done in parallel with topAlgorithm.execute)
        algorithm.execute(tree, TbfAlgorithmUtils::TbfTransferStages);
        // Top to bottom
        algorithm.execute(tree, TbfAlgorithmUtils::TbfTopToBottomStages);

        timerExecute.stop();
        std::cout << "Execute in " << timerExecute.getElapsed() << std::endl;

        std::cout << "Perform the periodic FMM with parameters:" << std::endl;
        std::cout << " - idxExtraLevel: " << idxExtraLevel << std::endl;
        std::cout << " - Nb repeat per dim: " << topAlgorithm.getNbRepetitionsPerDim() << std::endl;
        std::cout << " - Number of times the real box is duplicated: " << topAlgorithm.getNbTotalRepetitions() << std::endl;
        std::cout << " - Repeatition interves: " << TbfUtils::ArrayPrinter(topAlgorithm.getRepetitionsIntervals().first)
                  << " " << TbfUtils::ArrayPrinter(topAlgorithm.getRepetitionsIntervals().second) << std::endl;
        std::cout << " - original configuration: " << configuration << std::endl;
        std::cout << " - top tree configuration: " << TopPeriodicAlgorithmClass::GenerateAboveTreeConfiguration(configuration,idxExtraLevel) << std::endl;

        // Print the counter's result
        {
            std::cout << "Counter for the regular FMM (periodic):" << std::endl;
            auto counters = typename KernelClass::ReduceType();
            algorithm.applyToAllKernels([&](const auto& inKernel){
                counters = KernelClass::ReduceType::Reduce(counters, inKernel.getReduceData());
            });
            std::cout << counters << std::endl;
        }
        {
            std::cout << "Counter for the extra FMM at the top:" << std::endl;
            auto counters = typename KernelClass::ReduceType();
            topAlgorithm.applyToAllKernels([&](const auto& inKernel){
                counters = KernelClass::ReduceType::Reduce(counters, inKernel.getReduceData());
            });
            std::cout << counters << std::endl;
        }
    }

    return 0;
}

