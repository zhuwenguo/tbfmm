#include "tbfglobal.hpp"
#include "spacial/tbfmortonspaceindex.hpp"
#include "spacial/tbfspacialconfiguration.hpp"
#include "utils/tbfrandom.hpp"
#include "core/tbfcellscontainer.hpp"
#include "core/tbfparticlescontainer.hpp"
#include "core/tbfparticlesorter.hpp"
#include "core/tbftree.hpp"
#include "algorithms/sequential/tbfalgorithm.hpp"
#include "algorithms/smspetabaru/tbfsmspetabarualgorithm.hpp"
#include "utils/tbftimer.hpp"


#include <iostream>


template <class RealType_T, class SpaceIndexType_T = TbfDefaultSpaceIndexType<RealType_T>>
class KernelExample{
public:
    using RealType = RealType_T;
    using SpaceIndexType = SpaceIndexType_T;
    using SpacialConfiguration = TbfSpacialConfiguration<RealType, SpaceIndexType::Dim>;
public:
    /// This is called from the main to create the first kernel
    /// this can be adapt to pass what you need and fill the class's attributes
    explicit KernelExample(const SpacialConfiguration& /*inConfiguration*/){}

    /// This is is called from the FMM algorithm to have one kernel per thread
    /// the access will be thread safe (only one kernel is copied at a time)
    /// So if the kernel needs a huge constant matrix, one could use a mutable
    /// shared pointer and safely update it in this copy function
    explicit KernelExample(const KernelExample&){}

    template <class ParticlesClass, class LeafClass>
    void P2M(const typename SpaceIndexType::IndexType& /*inLeafIndex*/,
             const ParticlesClass&& /*inParticles*/, const long int /*inNbParticles*/, LeafClass& /*inOutLeaf*/) const {
        /// inLeafIndex: is the spacial index (Morton index for example) of the current leaf
        /// inParticles: is an array of pointers on the particles' data (not on the RHS)
        ///  - For i from 0 to Dim-1, inParticles[i][idxPart] gives the position of particle idxPart on axis i
        ///  - Then, for Dim <= i < NbData inParticles[i][idxPart] gives the data value for particle idxPart
        /// inNbParticles: is the number of particles
        /// inOutLeaf: is the multipole part (here MultipoleClass defined in the main)
    }

    template <class CellClassContainer, class CellClass>
    void M2M(const typename SpaceIndexType::IndexType& /*inCellIndex*/,
             const long int /*inLevel*/, const CellClassContainer& /*inLowerCell*/, CellClass& /*inOutUpperCell*/,
             const long int /*childrenPos*/[], const long int /*inNbChildren*/) const {
        /// inCellIndex: is the spacial index (Morton index for example) of the parent
        /// inLevel: the level in the tree of the parent
        /// inLowerCell: a container over the multipole parts of the children (~ an array of MultipoleClass
        ///              defined in the main)
        ///              const CellClass& child = inLowerCell[idxChild]
        /// inOutUpperCell: the parent multipole data.
        /// childrenPos: the position of each child. In 3D each index will be between 0 and 7 (as there
        ///              are 2^3 children).
        ///              This could be use to quickly retreive an transfer matrix, for example, one could
        ///              have an array "mat[8]" and always use "mat[idxChild]" for the children at the same
        ///              relative position.
        /// inNbChildren: the number of children.
        ///
        /// Please, note that the M2M might be called multiple time on the same parent. For instance,
        /// if the children are stored on different blocks, there will be one call per block.
    }

    template <class CellClassContainer, class CellClass>
    void M2L(const typename SpaceIndexType::IndexType& /*inCellIndex*/,
             const long int /*inLevel*/, const CellClassContainer& /*inInteractingCells*/,
             const long int /*neighPos*/[], const long int /*inNbNeighbors*/,
             CellClass& /*inOutCell*/) const {
        /// inCellIndex: is the spacial index (Morton index for example) of the target cell.
        ///              The indexes of the neighbors could be found from inCellIndex and neighPos.
        /// inLevel: the level in the tree of the parent
        /// inInteractingCells: a container over the multipole parts of the children (~ an array of MultipoleClass
        ///                     defined in the main)
        ///                     const CellClass& neigh = inInteractingCells[idxNeigh]
        /// neighPos: the position of each neigh. In 3D each index will be between 0 and 189.
        ///              This could be use to quickly retreive an transfer matrix, for example, one could
        ///              have an array "mat[189]" and always use "mat[idxNeigh]" for the children at the same
        ///              relative position.
        /// inNbNeighbors: the number of neighbors.
        /// inOutCell: the parent multipole data.
        ///
        /// Please, note that the M2L might be called multiple time on the target cell. For instance,
        /// if the children are stored on different blocks, there will be one call per block.
    }

    template <class CellClass, class CellClassContainer>
    void L2L(const typename SpaceIndexType::IndexType& /*inCellIndex*/,
             const long int /*inLevel*/, const CellClass& /*inUpperCell*/, CellClassContainer& /*inOutLowerCells*/,
             const long int /*childrednPos*/[], const long int /*inNbChildren*/) const {
        /// inCellIndex: is the spacial index (Morton index for example) of the parent
        /// inLevel: the level in the tree of the parent
        /// inUpperCell: the parent local data.
        /// inOutLowerCells: a container over the local parts of the children (~ an array of LocalClass
        ///              defined in the main)
        ///              const CellClass& child = inOutLowerCells[idxChild]
        /// childrenPos: the position of each child. In 3D each index will be between 0 and 7 (as there
        ///              are 2^3 children).
        ///              This could be use to quickly retreive an transfer matrix, for example, one could
        ///              have an array "mat[8]" and always use "mat[idxChild]" for the children at the same
        ///              relative position.
        /// inNbChildren: the number of children.
        ///
        /// Please, note that the L2L might be called multiple time on the same parent. For instance,
        /// if the children are stored on different blocks, there will be one call per block.
    }

    template <class LeafClass, class ParticlesClassValues, class ParticlesClassRhs>
    void L2P(const typename SpaceIndexType::IndexType& /*inCellIndex*/,
             const LeafClass& /*inLeaf*/,
             const ParticlesClassValues&& /*inOutParticles*/, ParticlesClassRhs&& /*inOutParticlesRhs*/,
             const long int /*inNbParticles*/) const {
        /// inCellIndex: is the spacial index (Morton index for example) of the current cell
        /// inLeaf: is the local part (here LocalClass defined in the main)
        /// inLeafIndex: is the spacial index (Morton index for example) of the current leaf
        /// inOutParticles: is an array of pointers on the particles' data (not on the RHS)
        ///  - For i from 0 to Dim-1, inOutParticles[i][idxPart] gives the position of particle idxPart on axis i
        ///  - Then, for Dim <= i < NbData inOutParticles[i][idxPart] gives the data value for particle idxPart
        /// inOutParticlesRhs: is an array of pointers on the particles' rhs
        ///  - Then, for 0 <= i < NbRhs inOutParticlesRhs[i][idxPart] gives the data value for particle idxPart
        /// inNbParticles: is the number of particles
    }

    template <class ParticlesClassValues, class ParticlesClassRhs>
    void P2P(const typename SpaceIndexType::IndexType& /*inNeighborIndex*/,
             const ParticlesClassValues&& /*inParticlesNeighbors*/, ParticlesClassRhs&& /*inParticlesNeighborsRhs*/,
             const long int /*inNbParticlesNeighbors*/,
             const typename SpaceIndexType::IndexType& /*inTargetIndex*/, const ParticlesClassValues&& /*inOutParticles*/,
             ParticlesClassRhs&& /*inOutParticlesRhs*/, const long int /*inNbOutParticles*/) const {
        /// To compute the interations between a leaf and a neighbor (should be done in both way).
        /// inNeighborIndex: is the spacial index (Morton index for example) of the neighbor
        /// inParticlesNeighbors: is an array of pointers on the particles' data (not on the RHS)
        ///  - For i from 0 to Dim-1, inOutParticles[i][idxPart] gives the position of particle idxPart on axis i
        ///  - Then, for Dim <= i < NbData inOutParticles[i][idxPart] gives the data value for particle idxPart
        /// inParticlesNeighborsRhs: is an array of pointers on the particles' rhs
        ///  - Then, for 0 <= i < NbRhs inOutParticlesRhs[i][idxPart] gives the data value for particle idxPart
        /// inNeighborPos: is the number of particles in the neighbor container
        /// inOutParticles: is an array of pointers on the particles' data (not on the RHS)
        ///  - For i from 0 to Dim-1, inOutParticles[i][idxPart] gives the position of particle idxPart on axis i
        ///  - Then, for Dim <= i < NbData inOutParticles[i][idxPart] gives the data value for particle idxPart
        /// inOutParticlesRhs: is an array of pointers on the particles' rhs
        ///  - Then, for 0 <= i < NbRhs inOutParticlesRhs[i][idxPart] gives the data value for particle idxPart
        /// inNbOutParticles: is the number of particles in the target container
    }

    template <class ParticlesClassValues, class ParticlesClassRhs>
    void P2PInner(const typename SpaceIndexType::IndexType& /*inSpacialIndex*/,
                  const ParticlesClassValues&& /*inOutParticles*/,
                  ParticlesClassRhs&& /*inOutParticlesRhs*/, const long int /*inNbOutParticles*/) const {
        /// To compute the interations inside a leaf.
        /// inSpacialIndex: is the spacial index (Morton index for example) of the neighbor
        /// inParticles: is an array of pointers on the particles' data (not on the RHS)
        ///  - For i from 0 to Dim-1, inParticles[i][idxPart] gives the position of particle idxPart on axis i
        ///  - Then, for Dim <= i < NbData inParticles[i][idxPart] gives the data value for particle idxPart
        /// inOutParticlesRhs: is an array of pointers on the particles' rhs
        ///  - Then, for 0 <= i < NbRhs inOutParticlesRhs[i][idxPart] gives the data value for particle idxPart
        /// inNbOutParticles: is the number of particles
    }
};


int main(){
    using RealType = double;
    const int Dim = 3;

    /////////////////////////////////////////////////////////////////////////////////////////

    const std::array<RealType, Dim> BoxWidths{{1, 1, 1}};
    const long int TreeHeight = 8;
    const std::array<RealType, Dim> inBoxCenter{{0.5, 0.5, 0.5}};

    const TbfSpacialConfiguration<RealType, Dim> configuration(TreeHeight, BoxWidths, inBoxCenter);

    /////////////////////////////////////////////////////////////////////////////////////////

    const long int NbParticles = 1000;

    TbfRandom<RealType, Dim> randomGenerator(configuration.getBoxWidths());

    std::vector<std::array<RealType, Dim>> particlePositions(NbParticles);

    for(long int idxPart = 0 ; idxPart < NbParticles ; ++idxPart){
        particlePositions[idxPart] = randomGenerator.getNewItem();
    }

    /////////////////////////////////////////////////////////////////////////////////////////

    constexpr long int NbDataValuesNeeded = 0; // TODO how many real values you need in the data part
    constexpr long int NbDataValuesPerParticle = Dim;
    using ParticleRhsType = double; // TODO what type are the particle RHS
    constexpr long int NbRhsValuesPerParticle = 1; // TODO how many real values you need in the rhs part
    using MultipoleClass = std::array<RealType,1>; // TODO what is a multipole part, could be a class, but must be POD
    using LocalClass = std::array<RealType,1>; // TODO what is a local part, could be a class, but must be POD
    const long int inNbElementsPerBlock = 50;
    const bool inOneGroupPerParent = false;

    /////////////////////////////////////////////////////////////////////////////////////////

    TbfTimer timerBuildTree;

    TbfTree<RealType, RealType, NbDataValuesPerParticle, ParticleRhsType, NbRhsValuesPerParticle, MultipoleClass, LocalClass> tree(configuration, inNbElementsPerBlock,
                                                                                particlePositions, inOneGroupPerParent);

    timerBuildTree.stop();
    std::cout << "Build the tree in " << timerBuildTree.getElapsed() << std::endl;

    TbfAlgorithm<RealType, KernelExample<RealType>> algorithm(configuration);
    // TODO use a parallel version TbfSmSpetabaruAlgorithm<RealType, KernelExample<RealType>> algorithm(configuration);

    TbfTimer timerExecute;

    algorithm.execute(tree);

    timerExecute.stop();
    std::cout << "Execute in " << timerExecute.getElapsed() << std::endl;

    /////////////////////////////////////////////////////////////////////////////////////////

    tree.applyToAllLeaves([]
                          (auto&& leafHeader, const long int* particleIndexes,
                          const std::array<RealType*, NbDataValuesPerParticle> particleDataPtr,
                          const std::array<RealType*, NbRhsValuesPerParticle> particleRhsPtr){
        /// leafHeader.nbParticles: spacial index of the current cell
        /// particleIndexes: indexes of the particles, this correspond to the original order when
        ///                  creating the tree.
        /// particleDataPtr: array of pointers of the particles' data
        /// particleRhsPtr: array of pointers of the particles' rhs
    });

    return 0;
}
