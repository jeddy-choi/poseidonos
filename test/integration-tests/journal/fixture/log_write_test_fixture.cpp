#include "test/integration-tests/journal/fixture/log_write_test_fixture.h"
#include "test/integration-tests/journal/utils/used_offset_calculator.h"
#include "test/integration-tests/journal/fake/test_journal_write_completion.h"

namespace pos
{
LogWriteTestFixture::LogWriteTestFixture(MockMapper* _mapper, ArrayInfoMock* _array,
    JournalManagerSpy* _journal, TestInfo* _testInfo)
{
    Reset();

    mapper = _mapper;
    array = _array;
    journal = _journal;
    testInfo = _testInfo;
    rbaGenerator = new RbaGenerator(testInfo);
}

LogWriteTestFixture::~LogWriteTestFixture(void)
{
    Reset();
    delete rbaGenerator;
}

void
LogWriteTestFixture::Reset(void)
{
    testingLogs.Reset();
    dirtyPages.clear();
}

void
LogWriteTestFixture::UpdateJournal(JournalManagerSpy* _journal)
{
    journal = _journal;
}

void
LogWriteTestFixture::WriteLogsWithSize(uint64_t sizeToFill)
{
    UsedOffsetCalculator usedOffset(journal, sizeToFill);

    while (1)
    {
        StripeId vsid = std::rand() % testInfo->numUserStripes;
        StripeTestFixture stripe(vsid, testInfo->defaultTestVol);
        auto blksToWrite = _GenerateBlocksInStripe(stripe.GetVsid(), 0, testInfo->numBlksPerStripe);

        for (auto blk : blksToWrite)
        {
            if (usedOffset.CanBeWritten(sizeof(BlockWriteDoneLog)) == true)
            {
                BlkAddr rba = std::get<0>(blk);
                VirtualBlks vsas = std::get<1>(blk);

                bool writeSuccessful = WriteBlockLog(testInfo->defaultTestVol, rba, vsas);
                EXPECT_TRUE(writeSuccessful == true);

                stripe.AddBlockMap(rba, vsas);
            }
            else
            {
                return;
            }
        }

        if (usedOffset.CanBeWritten(sizeof(StripeMapUpdatedLog)) == true)
        {
            // Write stripe map updated log only after all previous
            // block map updated log is written
            WaitForAllLogWriteDone();

            bool writeSuccessful = WriteStripeLog(vsid, stripe.GetWbAddr(), stripe.GetUserAddr());
            EXPECT_TRUE(writeSuccessful == true);
        }
        else
        {
            return;
        }
    }
}

bool
LogWriteTestFixture::WriteBlockLog(int volId, BlkAddr rba, VirtualBlks blks)
{
    MpageList dirty = (mapper->GetVSAMapMock())->GetDirtyVsaMapPages(volId, rba, blks.numBlks);
    uint32_t numBlksInSector = blks.numBlks * SECTORS_PER_BLOCK;
    StripeAddr stripeAddr = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = StripeTestFixture::GetWbLsid(blks.startVsa.stripeId)};

    VolumeIoSmartPtr volumeIo(new VolumeIo(nullptr, numBlksInSector, array->GetIndex()));
    volumeIo->SetSectorRba(ChangeBlockToSector(rba));
    volumeIo->SetVolumeId(volId);
    volumeIo->SetVsa(blks.startVsa);
    volumeIo->SetLsidEntry(stripeAddr);

    EventSmartPtr event(new TestJournalWriteCompletion(&testingLogs));
    int result = journal->AddBlockMapUpdatedLog(volumeIo, dirty, event);
    if (result == 0)
    {
        testingLogs.AddToWriteList(volumeIo);
        _AddToDirtyPageList(volId, dirty);
    }
    else
    {
        cout << "Log write failed" << endl;
    }

    return (result >= 0);
}

bool
LogWriteTestFixture::WriteStripeLog(StripeId vsid, StripeAddr oldAddr, StripeAddr newAddr)
{
    assert(newAddr.stripeLoc == IN_USER_AREA);

    MpageList dirty = (mapper->GetStripeMapMock())->GetDirtyStripeMapPages(vsid);

    Stripe* stripe = new Stripe(nullptr, true);
    stripe->SetVsid(vsid);
    stripe->SetUserLsid(newAddr.stripeId);

    EventSmartPtr event(new TestJournalWriteCompletion(&testingLogs));
    int result = journal->AddStripeMapUpdatedLog(stripe, oldAddr, dirty, event);
    if (result == 0)
    {
        testingLogs.AddToWriteList(stripe, oldAddr);
        _AddToDirtyPageList(STRIPE_MAP_ID, dirty);
    }
    else
    {
        cout << "Log write failed " << endl;
    }

    delete stripe;

    return (result >= 0);
}

bool
LogWriteTestFixture::WriteGcStripeLog(int volumeId, StripeId vsid, StripeId wbLsid, StripeId userLsid)
{
    GcStripeMapUpdateList mapUpdates;
    mapUpdates.volumeId = volumeId;
    mapUpdates.vsid = vsid;
    mapUpdates.wbLsid = wbLsid;
    mapUpdates.userLsid = userLsid;
    MapPageList dirtyMap;

    _GenerateGcBlockLogs(mapUpdates, dirtyMap);

    EventSmartPtr event(new TestJournalWriteCompletion(&testingLogs));
    int result = journal->AddGcStripeFlushedLog(mapUpdates, dirtyMap, event);
    if (result == 0)
    {
        testingLogs.AddToWriteList(mapUpdates);
        _AddToDirtyPageList(volumeId, dirtyMap[volumeId]);
        _AddToDirtyPageList(STRIPE_MAP_ID, dirtyMap[STRIPE_MAP_ID]);
    }
    else
    {
        cout << "Log write failed " << endl;
    }

    return (result >= 0);
}

bool
LogWriteTestFixture::WriteGcStripeLog(int volumeId, StripeTestFixture& stripe)
{
    GcStripeMapUpdateList mapUpdates;
    mapUpdates.volumeId = volumeId;
    mapUpdates.vsid = stripe.GetVsid();
    mapUpdates.wbLsid = stripe.GetWbAddr().stripeId;
    mapUpdates.userLsid = stripe.GetUserAddr().stripeId;
    MapPageList dirtyMap;

    _GenerateGcBlockLogs(mapUpdates, dirtyMap);

    EventSmartPtr event(new TestJournalWriteCompletion(&testingLogs));
    int result = journal->AddGcStripeFlushedLog(mapUpdates, dirtyMap, event);
    if (result == 0)
    {
        testingLogs.AddToWriteList(mapUpdates);
        _AddToDirtyPageList(volumeId, dirtyMap[volumeId]);
        _AddToDirtyPageList(STRIPE_MAP_ID, dirtyMap[STRIPE_MAP_ID]);
    }
    else
    {
        cout << "Log write failed " << endl;
    }

    for (auto map : mapUpdates.blockMapUpdateList)
    {
        VirtualBlks blks = {
            .startVsa = map.vsa,
            .numBlks = 1};
        stripe.AddBlockMap(map.rba, blks);
    }

    return (result >= 0);
}

void
LogWriteTestFixture::_GenerateGcBlockLogs(GcStripeMapUpdateList& mapUpdates, MapPageList& dirtyMap)
{
    MpageList volumeDirtyList;
    for (BlkOffset offset = 0; offset < testInfo->numBlksPerStripe; offset++)
    {
        GcBlockMapUpdate update = {
            .rba = offset,
            .vsa = {
                .stripeId = mapUpdates.vsid,
                .offset = offset}};
        mapUpdates.blockMapUpdateList.push_back(update);
        auto dirty = mapper->GetVSAMapMock()->GetDirtyVsaMapPages(mapUpdates.volumeId, update.rba, 1);
        volumeDirtyList.insert(dirty.begin(), dirty.end());
    }

    dirtyMap[mapUpdates.volumeId] = volumeDirtyList;
    dirtyMap[STRIPE_MAP_ID] = (mapper->GetStripeMapMock())->GetDirtyStripeMapPages(mapUpdates.vsid);
}

void
LogWriteTestFixture::_AddToDirtyPageList(int mapId, MpageList dirty)
{
    std::lock_guard<std::mutex> lock(dirtyListLock);

    if (dirtyPages.find(mapId) == dirtyPages.end())
    {
        dirtyPages.emplace(mapId, dirty);
    }
    else
    {
        dirtyPages[mapId].insert(dirty.begin(), dirty.end());
    }
}

void
LogWriteTestFixture::GenerateLogsForStripe(StripeTestFixture& stripe, uint32_t startOffset, int numBlks)
{
    BlockMapList blksToWrite = _GenerateBlocksInStripe(stripe.GetVsid(), startOffset, numBlks);
    for (auto blk : blksToWrite)
    {
        BlkAddr rba = std::get<0>(blk);
        VirtualBlks vsas = std::get<1>(blk);

        stripe.AddBlockMap(rba, vsas);
    }
}

void
LogWriteTestFixture::WriteLogsForStripe(StripeTestFixture& stripe)
{
    WriteBlockLogsForStripe(stripe);

    WaitForAllLogWriteDone();

    bool writeSuccessful = WriteStripeLog(stripe.GetVsid(), stripe.GetWbAddr(), stripe.GetUserAddr());
    EXPECT_TRUE(writeSuccessful == true);
}

void
LogWriteTestFixture::WaitForAllLogWriteDone(void)
{
    testingLogs.WaitForAllLogWriteDone();
}

// TODO (huijeong.kim) block offset of vsa to be uint64_t
BlockMapList
LogWriteTestFixture::_GenerateBlocksInStripe(StripeId vsid, uint32_t startOffset, int numBlks)
{
    BlockMapList listToReturn;

    int blksRemaining = numBlks;
    VirtualBlkAddr vsa = {.stripeId = vsid, .offset = startOffset};

    while (blksRemaining != 0)
    {
        vsa.offset = numBlks - blksRemaining + startOffset;

        uint32_t blksToWrite;
        if (blksRemaining == 1)
        {
            blksToWrite = 1;
        }
        else
        {
            blksToWrite = blksRemaining / 2;
        }

        VirtualBlks blks = {.startVsa = vsa, .numBlks = blksToWrite};

        BlkAddr rba = rbaGenerator->Generate(blksToWrite);
        listToReturn.push_back(std::make_pair(rba, blks));
        blksRemaining -= blksToWrite;
    }

    return listToReturn;
}

void
LogWriteTestFixture::WriteOverwrittenBlockLogs(StripeTestFixture& stripe,
    BlkAddr rba, uint32_t startOffset, uint32_t numOverwrites)
{
    for (uint32_t logCount = 0; logCount < numOverwrites; logCount++)
    {
        VirtualBlkAddr vsa = {
            .stripeId = stripe.GetVsid(),
            .offset = startOffset + logCount};
        VirtualBlks blks = {.startVsa = vsa, .numBlks = 1};

        bool writeSuccessful = WriteBlockLog(testInfo->defaultTestVol, rba, blks);
        EXPECT_TRUE(writeSuccessful == true);

        stripe.AddBlockMap(rba, blks);
    }
}

void
LogWriteTestFixture::WriteBlockLogsForStripe(StripeTestFixture& stripe)
{
    for (auto blk : stripe.GetBlockMapList())
    {
        BlkAddr rba = std::get<0>(blk);
        VirtualBlks vsas = std::get<1>(blk);
        bool writeSuccessful = WriteBlockLog(stripe.GetVolumeId(), rba, vsas);
        EXPECT_TRUE(writeSuccessful == true);
    }
}

bool
LogWriteTestFixture::AreAllLogWritesDone(void)
{
    return testingLogs.AreAllLogWritesDone();
}

void
LogWriteTestFixture::CompareLogs(void)
{
    LogList logList;
    EXPECT_TRUE(journal->GetLogs(logList) == 0);

    std::list<LogHandlerInterface*> readLogs = logList.GetLogs();
    EXPECT_EQ(testingLogs.GetNumLogsInTesting(), readLogs.size());

    while (readLogs.size() != 0)
    {
        LogHandlerInterface* log = readLogs.front();
        EXPECT_TRUE(testingLogs.CheckLogInTheList(log) == true);
        readLogs.pop_front();
    }

    EXPECT_TRUE(readLogs.size() == 0);
}

MapPageList
LogWriteTestFixture::GetDirtyMap(void)
{
    return dirtyPages;
}
} // namespace pos
