/*  Sirikata
 *  CassandraObjectFactory.hpp
 *
 *  Copyright (c) 2010, Ewen Cheslack-Postava
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
 *  * Neither the name of Sirikata nor the names of its contributors may
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

#ifndef _SIRIKATA_OH_CASSANDRA_OBJECT_FACTORY_HPP_
#define _SIRIKATA_OH_CASSANDRA_OBJECT_FACTORY_HPP_

#include <sirikata/oh/ObjectFactory.hpp>
#include <sirikata/oh/HostedObject.hpp>
#include <sirikata/oh/SimulationFactory.hpp>
#include <libcassandra/cassandra.h>

namespace Sirikata {

/** CassandraObjectFactory generates objects from an input Cassandra file. */
class CassandraObjectFactory : public ObjectFactory {
public:

    CassandraObjectFactory(ObjectHostContext* ctx, ObjectHost* oh, const SpaceID& space, const String& host, int port, const String& oh_id);
    virtual ~CassandraObjectFactory() {}

    virtual void generate(const String& timestamp="current");

private:
    typedef org::apache::cassandra::Column Column;
    typedef org::apache::cassandra::SliceRange SliceRange;

    void connectObjects();

    struct ObjectInfo {
        UUID id;
        String scriptType;
        String scriptArgs;
        String scriptContents;
    };

    ObjectHostContext* mContext;
    ObjectHost* mOH;
    SpaceID mSpace;
    String mDBHost;
    int mDBPort;
    String mOHostID;  // Object host ID
    int32 mConnectRate;
    typedef std::queue<ObjectInfo> ObjectInfoQueue;
    ObjectInfoQueue mIncompleteObjects;
};

} // namespace Sirikata

#endif //_SIRIKATA_OH_CASSANDRA_OBJECT_FACTORY_HPP_
