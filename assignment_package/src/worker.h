#ifndef WORKER_H
#define WORKER_H

#pragma once

#include <QRunnable>
#include <QThread>
#include "scene/terrain.h"
#include "scene/lsystem.h"

class Worker : public QRunnable
{
private:
    Terrain *m_terrain;
    LSystem *m_lsystem;
    Rect16 m_rect;
public:
    Worker(Terrain* terrain, LSystem *lsystem, const Rect16 &rect):
        m_terrain(terrain), m_lsystem(lsystem), m_rect(rect) {}
    void run() override;
};

class ThreadData
{
public:
    // determine whether the thread worker is running
    // 0: not running, 1: running, 2: just finished
    static int workerRunning;
    // the rect that the worker is running on
    static Rect16 workerRect;
    // chunk create infos
    static ChunkCreateInfo info;
    static ChunkCreateInfo linfo;
    static ChunkCreateInfo rinfo;
    static ChunkCreateInfo finfo;
    static ChunkCreateInfo binfo;
};

#endif // WORKER_H
