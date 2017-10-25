#include "Transforms/DataDistribution/MPI/MPIDataDistributionGather.h"

#include "Shared/DataShared.h"
#include "Shared/ExpressionHelpers.h"
#include "Shared/SubroutinesShared.h"
#include "OPS_Core/Localization.h"

namespace OPS
{
namespace Transforms
{
namespace DataDistribution
{
    using namespace OPS::Transforms::MPIProducer;
    using namespace OPS::Reprise;
    using namespace OPS::Shared::ExpressionHelpers;

    MPIDataDistributionGather::MPIDataDistributionGather(
        BADDParametersFamily distributionParameters, 
        BADDParameters generatorParameters,
        VariableDeclaration& targetArrayDeclaration, 
        StatementBase& insertionPointStatement, 
        int leadingDimention,
        int sourceNodeNumber):
        m_generatorParameters(generatorParameters),
        m_distributionParameters(distributionParameters),
        m_targetArrayDeclaration(targetArrayDeclaration),
        m_pNewArrayDeclaration(NULL),
        m_insertionPointStatement(insertionPointStatement),
        m_leadingDimention(leadingDimention),
        m_pRankDeclaration(NULL),
        m_sourceNodeNumber(sourceNodeNumber),
        m_analysisRerformed(false)
    {
        m_pFactory = new MPIProducerCFactory();

        m_pHelper = m_pFactory->createMPIHelper(Shared::getTranslationUnit(&m_insertionPointStatement));
        OPS_ASSERT(m_pHelper != NULL);

        m_pRankProducer = m_pFactory->createMPIRankProducer(Shared::getSubroutineDeclarationByStatement(&m_insertionPointStatement));
        OPS_ASSERT(m_pRankProducer != NULL);

        TypeBase* pArrayElementType = Shared::getArrayElementBasicType(&m_targetArrayDeclaration.getType());
        if(pArrayElementType == NULL) return;

        m_pArrayElementBasicType = pArrayElementType->cast_ptr<BasicType>();

        m_pMallocDeclaration = Shared::findSubroutineByName(Shared::getTranslationUnit(&m_insertionPointStatement), "malloc");
        m_pFreeDeclaration = Shared::findSubroutineByName(Shared::getTranslationUnit(&m_insertionPointStatement), "free");
    }

    bool MPIDataDistributionGather::analyseApplicability()
    {
        m_errors.clear();

        OPS_ASSERT(m_pHelper != NULL);
        if(!m_pHelper->validate())
        {
            m_errors.push_back(_TL("MPI library is not included or not correct.", "Библиотека MPI не подключена или некорректна."));
        }

        OPS_ASSERT(m_pRankProducer != NULL);
        if(!m_pRankProducer->analyseApplicability())
        {
            m_errors.push_back(m_pRankProducer->getErrorMessage());
        }

        if(m_pArrayElementBasicType == NULL)
        {
            m_errors.push_back(_TL("Elements of distributing array must be of basic type.", "Элементы распределяемого массива должны иметь базовый тип."));
        }

        if(m_pMallocDeclaration == NULL)
        {
            m_errors.push_back(_TL("Cannot find malloc subroutine.", "Не удалось найти функцию malloc."));
        }

        if (m_pFreeDeclaration == NULL)
        {
            m_errors.push_back(_TL("Cannot find free subroutine.", "Не удалось найти функцию free."));
        }

        if(m_leadingDimention < 0 || m_leadingDimention > (int)m_distributionParameters.dims.size())
        {
            m_errors.push_back(_TL("Wrong leading dimention", "Некорректное ведущее измерение"));
        }

        if(!m_generatorParameters.isValid())
        {
            m_errors.push_back(_TL("Invalid parameters of data distribution", "Некорректные параметры размещения данных"));
        }

        m_analysisRerformed = true;

        return m_errors.size() == 0;
    }

    std::string MPIDataDistributionGather::getErrorMessage()
    {
        std::string error = "";

        for(std::list<std::string>::iterator it = m_errors.begin(); it != m_errors.end(); ++it)
        {
            error = error + "\n" + *it;
        }

        return error;
    }

    void MPIDataDistributionGather::makeTransformation()
    {
        if(!m_analysisRerformed)
        {
            analyseApplicability();
        }
        OPS_ASSERT(m_errors.size() == 0);

        m_generatorParameters.normalize();

        OPS_ASSERT(m_pNewArrayDeclaration != NULL);

        // Заводим блок, в котором будут находиться все новые операторы
        ReprisePtr<BlockStatement> rpOutputBlock(new BlockStatement());
        m_insertionPointStatement.getParentBlock().addAfter(m_insertionPointStatement.getParentBlock().convertToIterator(&m_insertionPointStatement), rpOutputBlock.get());

        // Заводим переменную rank
        m_pRankDeclaration = MPIHelper::getRankDeclaration(*Shared::getSubroutineDeclarationByStatement(&m_insertionPointStatement));
        if(m_pRankDeclaration == NULL)
        {
            m_pRankProducer->makeTransformation();
            m_pRankDeclaration = MPIHelper::getRankDeclaration(*Shared::getSubroutineDeclarationByStatement(&m_insertionPointStatement));
        }
        
        // if(rank == <sourceNodeNumber>)
        ReprisePtr<IfStatement> rpIfStatement(new IfStatement());
        rpOutputBlock->addLast(rpIfStatement.get());
        IntegerHelper ihRank(m_pRankProducer->getRankType()->getKind());
        rpIfStatement->setCondition(&(ihRank(m_sourceNodeNumber % m_distributionParameters.P) == *(new ReferenceExpression(*m_pRankDeclaration))));
        ReprisePtr<BlockStatement> rpThenBlockOfIfStatement(new BlockStatement());
        ReprisePtr<BlockStatement> rpElseBlockOfIfStatement(new BlockStatement());
        rpIfStatement->setThenBody(rpThenBlockOfIfStatement.get());
        rpIfStatement->setElseBody(rpElseBlockOfIfStatement.get());

        // Блок ELSE
        // Вызов MPI_Send
        IntegerHelper ihCount(Shared::getArgumentPointedType(m_pHelper->getMPIFunctionDeclaration(MPIFunctionsHelper::MF_MPI_RECV), 1)->cast_to<BasicType>());
        IntegerHelper ihDest(Shared::getArgumentPointedType(m_pHelper->getMPIFunctionDeclaration(MPIFunctionsHelper::MF_MPI_RECV), 3)->cast_to<BasicType>());
        IntegerHelper ihTag(Shared::getArgumentPointedType(m_pHelper->getMPIFunctionDeclaration(MPIFunctionsHelper::MF_MPI_RECV), 4)->cast_to<BasicType>());
        rpElseBlockOfIfStatement->addLast(m_pHelper->createMPISendStatement(
            *(new ReferenceExpression(*m_pNewArrayDeclaration)),
            ihCount((sdword)m_distributionParameters.getElementsInEachProcessorCount(m_leadingDimention)),
            *m_pHelper->createMPIBasicTypeCode(m_pFactory->createMPIBasicTypesHelper()->getMPIBasicTypeByRepriseBasicType(m_pArrayElementBasicType->getKind())),
            ihDest(m_sourceNodeNumber % m_distributionParameters.P),
            ihTag(0),
            *m_pHelper->createMPICommCode(MPICommsHelper::MC_MPI_COMM_WORLD)
            ).get());
        // Конец блока ELSE

        // Блок THEN
        // Объявляем переменную типа MPI_Status для MPI_Recv
        TypeBase& statusType = *m_pHelper->getMPITypeDeclaration(MPITypesHelper::MT_MPI_STATUS);
        VariableDeclaration& status = Editing::createNewVariable(statusType, *rpThenBlockOfIfStatement, "_STATUS");

        std::list<VariableDeclaration*> buffers;
        std::list<VariableDeclaration*> requests;
        
        // Объявляем новые переменные-счетчики циклов.
        // Для массива с размерностью M создается M счетчиков для прохода по клеткам и M счетчиков для прохода по элементам внутри блока
        VariableContainer bigCellCounters(m_distributionParameters.dims.size());
        VariableContainer littleCellCounters(m_distributionParameters.dims.size());
        ReprisePtr<TypeBase> indexType(BasicType::int32Type());
        IntegerHelper indexIH(BasicType::BT_INT32);
        for(std::vector<int>::size_type i = 0; i < m_distributionParameters.dims.size(); ++i)
        {
            bigCellCounters[i] = &Editing::createNewVariable(*indexType, *rpThenBlockOfIfStatement, "_I_");
            littleCellCounters[i] = &Editing::createNewVariable(*indexType, *rpThenBlockOfIfStatement, "_i_");
        }

        BADDParametersFamily::MultyIndexList allBlocksInCellMultyIndeces = m_distributionParameters.getBlocksMultyIndecesInCell();
        
        // Блок, в котором будут MPI_IRecv
        ReprisePtr<BlockStatement> rpIRecvBlock(new BlockStatement());

        // Блок, в котором будет копирование данных
        ReprisePtr<BlockStatement> rpCopyBlock(new BlockStatement());
        
        // Принимаем данные от каждого узла кроме 0
        for(int processNumber = 0; processNumber < m_distributionParameters.P; ++processNumber)
        {
            VariableDeclaration* pRequestDeclaration = NULL;
            if(processNumber != m_sourceNodeNumber % m_distributionParameters.P)
            {
                pRequestDeclaration = &Editing::createNewVariable(*m_pHelper->getMPITypeDeclaration(MPITypesHelper::MT_MPI_REQUEST), *rpThenBlockOfIfStatement, "_REQUEST");
            }

            // Создаем буферы, в которые будем получать данные
            ReprisePtr<TypeBase> rpReceivingBufferType(new PtrType(m_pArrayElementBasicType));
            VariableDeclaration* receivingBufferDeclaration = NULL;
            if(processNumber == m_sourceNodeNumber % m_distributionParameters.P)
            {
                receivingBufferDeclaration = m_pNewArrayDeclaration;
            }
            else
            {
                receivingBufferDeclaration = &Editing::createNewVariable(*rpReceivingBufferType, *rpThenBlockOfIfStatement, "_SEND_BUF_");
                rpThenBlockOfIfStatement->addLast(createMemoryAllocatingStatement(*receivingBufferDeclaration, m_distributionParameters.getElementsInEachProcessorCount(m_leadingDimention), *m_pArrayElementBasicType).get());
                buffers.push_back(receivingBufferDeclaration);

                // Вызов MPI_IRecv
                IntegerHelper ihCount(Shared::getArgumentPointedType(m_pHelper->getMPIFunctionDeclaration(MPIFunctionsHelper::MF_MPI_RECV), 1)->cast_to<BasicType>());
                IntegerHelper ihSource(Shared::getArgumentPointedType(m_pHelper->getMPIFunctionDeclaration(MPIFunctionsHelper::MF_MPI_RECV), 3)->cast_to<BasicType>());
                IntegerHelper ihTag(Shared::getArgumentPointedType(m_pHelper->getMPIFunctionDeclaration(MPIFunctionsHelper::MF_MPI_RECV), 4)->cast_to<BasicType>());
                rpIRecvBlock->addLast(m_pHelper->createMPIIRecvStatement(
                    *(new ReferenceExpression(*receivingBufferDeclaration)),
                    ihCount((sdword)m_distributionParameters.getElementsInEachProcessorCount(m_leadingDimention)),
                    *m_pHelper->createMPIBasicTypeCode(m_pFactory->createMPIBasicTypesHelper()->getMPIBasicTypeByRepriseBasicType(m_pArrayElementBasicType->getKind())),
                    ihSource((sdword)processNumber),
                    ihTag(0),
                    *m_pHelper->createMPICommCode(MPICommsHelper::MC_MPI_COMM_WORLD),
                    R_AD()*(new ReferenceExpression(*pRequestDeclaration))
                    ).get());
            }
            if(processNumber != m_sourceNodeNumber % m_distributionParameters.P)
            {
                rpCopyBlock->addLast(m_pHelper->createMPIWaitStatement(
                    R_AD()*(new ReferenceExpression(*pRequestDeclaration)),
                    R_AD()*(new ReferenceExpression(status))).get());
            }

            // Гнездо циклов по клеткам
            BlockStatement* pBlockToAddForStatement = rpCopyBlock.get();
            for(VariableContainer::size_type i = 0; i < bigCellCounters.size(); ++i)
            {
                ReprisePtr<ForStatement> rpForStatement(new ForStatement());
                rpForStatement->setInitExpression(new BasicCallExpression(BasicCallExpression::BCK_ASSIGN, new ReferenceExpression(*bigCellCounters[i]), &indexIH(0)));
                rpForStatement->setFinalExpression(new BasicCallExpression(BasicCallExpression::BCK_LESS, new ReferenceExpression(*bigCellCounters[i]), &indexIH(m_distributionParameters.getWidthInCells(i))));
                rpForStatement->setStepExpression(new BasicCallExpression(BasicCallExpression::BCK_ASSIGN, new ReferenceExpression(*bigCellCounters[i]), &(*(new ReferenceExpression(*bigCellCounters[i])) + indexIH(1))));

                pBlockToAddForStatement->addLast(rpForStatement.get());
                pBlockToAddForStatement = &rpForStatement->getBody();
            }

            // Гнездо циклов по элементам внутри блока. 
            for(VariableContainer::size_type i = 0; i < littleCellCounters.size(); ++i)
            {
                ReprisePtr<ForStatement> rpForStatement(new ForStatement());
                rpForStatement->setInitExpression(new BasicCallExpression(BasicCallExpression::BCK_ASSIGN, new ReferenceExpression(*littleCellCounters[i]), &indexIH(0)));
                rpForStatement->setFinalExpression(new BasicCallExpression(BasicCallExpression::BCK_LESS, new ReferenceExpression(*littleCellCounters[i]), &indexIH(m_distributionParameters.d[i])));
                rpForStatement->setStepExpression(new BasicCallExpression(BasicCallExpression::BCK_ASSIGN, new ReferenceExpression(*littleCellCounters[i]), &(*(new ReferenceExpression(*littleCellCounters[i])) + indexIH(1))));

                pBlockToAddForStatement->addLast(rpForStatement.get());
                pBlockToAddForStatement = &rpForStatement->getBody();
            }

            // Мультииндексы тех блоков которые необходимо отослать процессу с номером processNumber
            BADDParametersFamily::MultyIndexList allBlocksInCellOfProcessorMultyIndeces = m_distributionParameters.getBlocksMultyIndecesOfProcessInCell(allBlocksInCellMultyIndeces, processNumber);

            // Для каждого из таких блоков нужно скопировать данные
            for(BADDParametersFamily::MultyIndexList::iterator it = allBlocksInCellOfProcessorMultyIndeces.begin(); it != allBlocksInCellOfProcessorMultyIndeces.end(); ++it)
            {
                // Формируем элемент, в который копируем
                ReprisePtr<ExpressionBase> rpSource(new ReferenceExpression(m_targetArrayDeclaration));
                for(BADDParametersFamily::ParametersContainer::size_type i = 0; i < m_distributionParameters.dims.size(); ++i)
                {
                    ExpressionBase* pIndex = NULL;

                    if(m_generatorParameters.a[i] != 0)
                    {
                        pIndex = &(
                            (
                                *(new ReferenceExpression(*bigCellCounters[i])) * indexIH(sqword(m_generatorParameters.getCellWidthInBlocks(i) * m_generatorParameters.d[i])) +
                                indexIH((*it)[i] * m_generatorParameters.d[i]) +
                                *(new ReferenceExpression(*littleCellCounters[i])) + indexIH(m_generatorParameters.a[i]) +
                                indexIH(m_generatorParameters.dims[i])
                            ) % indexIH(m_generatorParameters.dims[i])
                        );
                    }
                    else
                    {
                        pIndex = &(
                            *(new ReferenceExpression(*bigCellCounters[i])) * indexIH(sqword(m_generatorParameters.getCellWidthInBlocks(i) * m_generatorParameters.d[i])) +
                            indexIH((*it)[i] * m_generatorParameters.d[i]) +
                            *(new ReferenceExpression(*littleCellCounters[i]))
                        );
                    }

                    // Формируем индекс
                    ReprisePtr<ExpressionBase> rpIndex(pIndex);
                    rpSource.reset(new BasicCallExpression(BasicCallExpression::BCK_ARRAY_ACCESS, rpSource.get(), rpIndex.get()));
                }

                // Формируем элемент, который копируем
                ReprisePtr<ExpressionBase> rpCellOrdinalNumber(new ReferenceExpression(*bigCellCounters[bigCellCounters.size() - 1]));
                int multiplier = m_generatorParameters.getWidthInCells(bigCellCounters.size() - 1);
                for(BADDParametersFamily::ParametersContainer::size_type i = 1; i < m_distributionParameters.dims.size(); ++i)
                {
                    rpCellOrdinalNumber.reset(&(*rpCellOrdinalNumber + indexIH(multiplier) * *(new ReferenceExpression(*bigCellCounters[bigCellCounters.size() - i - 1]))));
                    multiplier = multiplier * m_distributionParameters.getWidthInCells(bigCellCounters.size() - i - 1);
                }
                ReprisePtr<ExpressionBase> rpElementInBlockOrdinalNumber(
                    &(
                        *(new ReferenceExpression(*littleCellCounters[littleCellCounters.size() - 1])) +
                        indexIH(m_distributionParameters.leftOverlapping[littleCellCounters.size() - 1] + m_generatorParameters.a[littleCellCounters.size() - 1])
                    )
                );
                multiplier = m_distributionParameters.d[littleCellCounters.size() - 1] + m_distributionParameters.leftOverlapping[littleCellCounters.size() - 1] + m_distributionParameters.rightOverlapping[littleCellCounters.size() - 1];
                for(BADDParametersFamily::ParametersContainer::size_type i = 1; i < m_distributionParameters.dims.size(); ++i)
                {
                    rpElementInBlockOrdinalNumber.reset(
                        &(
                            *rpElementInBlockOrdinalNumber + 
                            indexIH(multiplier) * 
                            (
                                *(new ReferenceExpression(*littleCellCounters[littleCellCounters.size() - i - 1])) +
                                indexIH(m_distributionParameters.leftOverlapping[littleCellCounters.size() - i - 1] + m_generatorParameters.a[littleCellCounters.size() - i - 1])
                            )
                        )
                    );
                    multiplier = multiplier * (m_distributionParameters.d[littleCellCounters.size() - i - 1] + m_distributionParameters.leftOverlapping[littleCellCounters.size() - i - 1] + m_distributionParameters.rightOverlapping[littleCellCounters.size() - i - 1]);
                }
                ExpressionBase* pDestIndex = NULL;
                if(m_distributionParameters.dims.size() == 1)
                {
                    pDestIndex = &(
                        *rpCellOrdinalNumber * indexIH(m_distributionParameters.getCellWidthInBlocks(m_leadingDimention)) * indexIH(m_distributionParameters.getElementsInBlockCount()) +
                        *rpElementInBlockOrdinalNumber
                        );
                }
                else
                {
                    pDestIndex = &(
                        *rpCellOrdinalNumber * indexIH(m_distributionParameters.getCellWidthInBlocks(m_leadingDimention)) * indexIH(m_distributionParameters.getElementsInBlockCount()) +
                        indexIH((*it)[m_leadingDimention]) * indexIH(m_distributionParameters.getElementsInBlockCount()) +
                        *rpElementInBlockOrdinalNumber
                        );
                }
                ReprisePtr<ExpressionBase> rpDestIndex(pDestIndex);
                ReprisePtr<ExpressionBase> rpDest(new BasicCallExpression(BasicCallExpression::BCK_ARRAY_ACCESS, new ReferenceExpression(*receivingBufferDeclaration), rpDestIndex.get()));

                pBlockToAddForStatement->addLast(new ExpressionStatement(new BasicCallExpression(BasicCallExpression::BCK_ASSIGN, rpSource.get(), rpDest.get())));
            }
            
            if(processNumber != m_sourceNodeNumber % m_distributionParameters.P)
            {
                rpCopyBlock->addLast(createMemoryDeallocatingStatement(*receivingBufferDeclaration).get());
            }
        }

        rpThenBlockOfIfStatement->addLast(rpIRecvBlock.get());
        rpThenBlockOfIfStatement->addLast(rpCopyBlock.get());

        // Конец блока THEN
    }

    MPIDataDistributionGather::~MPIDataDistributionGather()
    {
        delete m_pHelper;
        delete m_pRankProducer;
        delete m_pFactory;
    }

    ReprisePtr<StatementBase> MPIDataDistributionGather::createMemoryAllocatingStatement(VariableDeclaration& variableDeclaration, int sizeInElements, BasicType& typeOfElements)
    {
        OPS_ASSERT(m_pMallocDeclaration != NULL);
        
        ReprisePtr<SubroutineReferenceExpression> rpMallocReferenceExpression(new SubroutineReferenceExpression(*m_pMallocDeclaration));
        ReprisePtr<SubroutineCallExpression> rpMallocCallExpression(new SubroutineCallExpression(rpMallocReferenceExpression.get()));

        IntegerHelper ih(BasicType::BT_INT32);
        ReprisePtr<ExpressionBase> rpSizeExpression(&(ih(sizeInElements * typeOfElements.getSizeOf())));

        rpMallocCallExpression->addArgument(rpSizeExpression.get());

        ReprisePtr<ExpressionBase> rpAssign(new BasicCallExpression(
            BasicCallExpression::BCK_ASSIGN, 
            new ReferenceExpression(variableDeclaration), 
            new TypeCastExpression(new PtrType(&typeOfElements), rpMallocCallExpression.get())));
        
        ReprisePtr<StatementBase> result(new ExpressionStatement(rpAssign.get()));

        return result;
    }

    ReprisePtr<StatementBase> MPIDataDistributionGather::createMemoryDeallocatingStatement(VariableDeclaration& variableDeclaration)
    {
        OPS_ASSERT(m_pFreeDeclaration != NULL);
        
        ReprisePtr<SubroutineReferenceExpression> rpFreeReferenceExpression(new SubroutineReferenceExpression(*m_pFreeDeclaration));
        ReprisePtr<SubroutineCallExpression> rpFreeCallExpression(new SubroutineCallExpression(rpFreeReferenceExpression.get()));

        rpFreeCallExpression->addArgument(new ReferenceExpression(variableDeclaration));
        ReprisePtr<StatementBase> result(new ExpressionStatement(rpFreeCallExpression.get()));

        return result;
    }

    void MPIDataDistributionGather::setNewArrayDeclaration( VariableDeclaration& newArrayDeclaration )
    {
        m_pNewArrayDeclaration = &newArrayDeclaration;
    }
}
}
}
