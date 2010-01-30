/*  cbr
 *  main.cpp
 *
 *  Copyright (c) 2009, Ewen Cheslack-Postava
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of cbr nor the names of its contributors may
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Timer.hpp"
#include "TimeSync.hpp"
#include "TimeProfiler.hpp"

#include "Options.hpp"
#include "Statistics.hpp"
#include "TabularServerIDMap.hpp"
#include "DistributedCoordinateSegmentation.hpp"

int main(int argc, char** argv) {
    using namespace CBR;

    InitOptions();
    ParseOptions(argc, argv);

    ServerID server_id = GetOption("id")->as<ServerID>();
    String trace_file = GetPerServerFile(STATS_TRACE_FILE, server_id);
    Trace* trace = new Trace(trace_file);

    // Compute the starting date/time
    String start_time_str = GetOption("wait-until")->as<String>();
    Time start_time = start_time_str.empty() ? Timer::now() : Timer::getSpecifiedDate( start_time_str );
    start_time += GetOption("wait-additional")->as<Duration>();

    Duration duration = GetOption("duration")->as<Duration>();

    IOService* ios = IOServiceFactory::makeIOService();
    IOStrand* mainStrand = ios->createStrand();


    Time init_space_ctx_time = Time::null() + (Timer::now() - start_time);
    SpaceContext* space_context = new SpaceContext(server_id, ios, mainStrand, start_time, init_space_ctx_time, trace, duration);

    BoundingBox3f region = GetOption("region")->as<BoundingBox3f>();
    Vector3ui32 layout = GetOption("layout")->as<Vector3ui32>();

    uint32 max_space_servers = GetOption("max-servers")->as<uint32>();
    if (max_space_servers == 0)
      max_space_servers = layout.x * layout.y * layout.z;

    srand( GetOption("rand-seed")->as<uint32>() );

    String filehandle = GetOption("serverips")->as<String>();
    std::ifstream ipConfigFileHandle(filehandle.c_str());
    ServerIDMap * server_id_map = new TabularServerIDMap(ipConfigFileHandle);

    String cseg_type = GetOption(CSEG)->as<String>();
    CoordinateSegmentation* cseg = new DistributedCoordinateSegmentation(space_context, region, layout, max_space_servers, server_id_map);

    ///////////Go go go!! start of simulation/////////////////////

    srand(time(NULL));

    space_context->add(space_context);
    space_context->add(cseg);

    space_context->run(1);

    space_context->cleanup();

    if (GetOption(PROFILE)->as<bool>()) {
        space_context->profiler->report();
    }

    trace->prepareShutdown();

    delete cseg;

    trace->shutdown();
    delete trace;
    trace = NULL;

    delete space_context;
    space_context = NULL;

    delete mainStrand;
    IOServiceFactory::destroyIOService(ios);

    return 0;
}
