#include "worker.h"
#include <iostream>

void Worker::run()
{
    int x = m_rect.xmid();
    int z = m_rect.zmid();
    ThreadData::workerRect = m_rect;
    m_terrain->buildChunk(x, z);
    m_lsystem->update(m_rect);
    m_terrain->placeAssets(m_rect);
    Chunk* chunk = m_terrain->getChunk(m_rect.xmid(), m_rect.zmid());
    ChunkCreateInfo info;
    ChunkCreateInfo li;
    ChunkCreateInfo ri;
    ChunkCreateInfo fi;
    ChunkCreateInfo bi;
    chunk->populateInfo(&info);
    chunk->populateNeighbor(&li, &ri, &fi, &bi);
    ThreadData::info = info;
    ThreadData::linfo = li;
    ThreadData::rinfo = ri;
    ThreadData::finfo = fi;
    ThreadData::binfo = bi;
    ThreadData::workerRunning = 2;
}

int ThreadData::workerRunning = 0;
Rect16 ThreadData::workerRect = Rect16(0, 0);
ChunkCreateInfo ThreadData::info = ChunkCreateInfo();
ChunkCreateInfo ThreadData::linfo = ChunkCreateInfo();
ChunkCreateInfo ThreadData::rinfo = ChunkCreateInfo();
ChunkCreateInfo ThreadData::finfo = ChunkCreateInfo();
ChunkCreateInfo ThreadData::binfo = ChunkCreateInfo();
