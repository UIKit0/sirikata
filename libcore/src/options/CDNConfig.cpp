/*  Sirikata
 *  CDNConfig.cpp
 *
 *  Copyright (c) 2009, Patrick Horn
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

#include <sirikata/core/util/Standard.hh>

#include <sirikata/core/options/CDNConfig.hpp>

#include <sirikata/core/task/EventManager.hpp>

#include <sirikata/core/transfer/EventTransferManager.hpp>
#include <sirikata/core/transfer/LRUPolicy.hpp>
#include <sirikata/core/transfer/DiskCacheLayer.hpp>
#include <sirikata/core/transfer/MemoryCacheLayer.hpp>
#include <sirikata/core/transfer/NetworkCacheLayer.hpp>
#include <sirikata/core/transfer/HTTPDownloadHandler.hpp>
#include <sirikata/core/transfer/HTTPUploadHandler.hpp>
#include <sirikata/core/transfer/HTTPFormUploadHandler.hpp>
#include <sirikata/core/transfer/FileProtocolHandler.hpp>
#include <sirikata/core/transfer/HashNameHandler.hpp>
#include <sirikata/core/transfer/CachedNameLookupManager.hpp>
#include <sirikata/core/transfer/CachedServiceLookup.hpp>
#include <sirikata/core/transfer/ServiceManager.hpp>

namespace Sirikata {

using namespace Transfer;
using Task::GenEventManager;

cache_usize_type parseSize(const std::string &sizeStr) {
    std::istringstream stream (sizeStr);
    char units;
    double amount(0);
    stream >> amount >> units;
    units = tolower(units);
    switch (units) {
    case 'k':
        return cache_usize_type(amount * Transfer::kilobyte);
    case 'm':
        return cache_usize_type(amount * Transfer::megabyte);
    case 'g':
        return cache_usize_type(amount * Transfer::gigabyte);
    case 't':
        return cache_usize_type(amount * Transfer::terabyte);
    }
    return cache_usize_type(amount);
}

template <class CreatedType>
class OptionFactory {
public:
    typedef CreatedType* (*FactoryFunc) (const OptionMap&);
    typedef std::map<std::string, FactoryFunc> FactoryMap;
    FactoryMap factories;
    OptionFactory(void (*initializer) (OptionFactory<CreatedType> &)) {
        initializer(*this);
    }
    CreatedType *operator()(const OptionMap &options) {
        typename FactoryMap::const_iterator iter = factories.find(options.getValue());
        if (iter == factories.end())
            return NULL;
        return ((*iter).second)(options);
    }
    void insert(const std::string &name, FactoryFunc func) {
        factories.insert(typename FactoryMap::value_type(name,func));
    }
};

CachePolicy *createLRUPolicy(const OptionMap &options) {
    return new LRUPolicy(parseSize(options["size"].getValue()));
}

void initializePolicy(OptionFactory<CachePolicy> &factories) {
    factories.insert("LRU",&createLRUPolicy);
}
OptionFactory<CachePolicy> CreatePolicy(&initializePolicy);

CacheLayer *createMemoryCache(const OptionMap &options) {
    CachePolicy *policy = CreatePolicy(options["policy"]);
    if (!policy)
        return NULL;
    return new MemoryCacheLayer(policy, NULL);
}
CacheLayer *createDiskCache(const OptionMap &options) {
    CachePolicy *policy = CreatePolicy(options["policy"]);
    if (!policy)
        return NULL;
    return new DiskCacheLayer(policy, options["directory"].getValue(), NULL);
}
CacheLayer *createNetworkCache(const OptionMap &options); // Defined below.


void initializeLayer(OptionFactory<CacheLayer> &factories) {
    factories.insert("Memory",&createMemoryCache);
    factories.insert("Disk",&createDiskCache);
    factories.insert("Network",&createNetworkCache);
}
OptionFactory<CacheLayer> CreateCache(&initializeLayer);


Transfer::ProtocolRegistry<Transfer::DownloadHandler> downloadProtocolRegistry;
Transfer::ProtocolRegistry<Transfer::NameLookupHandler> nameProtocolRegistry;
Transfer::ProtocolRegistry<Transfer::UploadHandler> uploadProtocolRegistry;
Transfer::ProtocolRegistry<Transfer::NameUploadHandler> nameUploadProtocolRegistry;
void initializeProtocols() {
            std::tr1::shared_ptr<Transfer::HTTPDownloadHandler> httpHandler(new Transfer::HTTPDownloadHandler);
            std::tr1::shared_ptr<Transfer::HTTPUploadHandler> httpRestUploadHandler(new Transfer::HTTPUploadHandler);
            std::tr1::shared_ptr<Transfer::HTTPFormUploadHandler> httpFormUploadHandler(new Transfer::HTTPFormUploadHandler);
            std::tr1::shared_ptr<Transfer::FileProtocolHandler> fileHandler(new Transfer::FileProtocolHandler);
            std::tr1::shared_ptr<Transfer::FileNameHandler> fileNameHandler(new Transfer::FileNameHandler(fileHandler));
            std::tr1::shared_ptr<Transfer::ComputeHashNameHandler> computeHashNameHandler(new Transfer::ComputeHashNameHandler());
            std::tr1::shared_ptr<Transfer::FilenameHashNameHandler> returnHashNameHandler(new Transfer::FilenameHashNameHandler());
            downloadProtocolRegistry.setHandler("http", httpHandler);
            nameProtocolRegistry.setHandler("http", httpHandler);
            uploadProtocolRegistry.setHandler("http", httpFormUploadHandler);
            nameUploadProtocolRegistry.setHandler("http", httpFormUploadHandler);

            downloadProtocolRegistry.setHandler("rest", httpHandler);
            nameProtocolRegistry.setHandler("rest", httpHandler);
            uploadProtocolRegistry.setHandler("rest", httpRestUploadHandler);
            nameUploadProtocolRegistry.setHandler("rest", httpRestUploadHandler);

            downloadProtocolRegistry.setHandler("file", fileHandler);
            nameProtocolRegistry.setHandler("file", fileNameHandler);
            uploadProtocolRegistry.setHandler("file", fileHandler);
            nameUploadProtocolRegistry.setHandler("file", fileNameHandler);

            nameProtocolRegistry.setHandler("x-shasumofuri", computeHashNameHandler); // http: URIs -- don't know hash, so take hash of URI instead.
            nameProtocolRegistry.setHandler("x-hashed", returnHashNameHandler); // mhash: URIs -- know hash from the filename.
}


void insertOneService(const OptionMap &urioption, Transfer::ListOfServices *services) {
            Transfer::ServiceParams params;
            for (OptionMap::const_iterator paramiter = urioption.begin(); paramiter != urioption.end(); ++paramiter) {
                params.insert(Transfer::ServiceParams::value_type((*paramiter).first, (*paramiter).second->getValue()));
            }
            services->push_back(Transfer::ListOfServices::value_type(
                                Transfer::URIContext(urioption.getValue()),
                                params));
}
void insertServices(const OptionMap &options, Transfer::CachedServiceLookup *serviceCache) {
    for (OptionMap::const_iterator iter = options.begin(); iter != options.end(); ++iter) {
        Transfer::ListOfServices *services = new Transfer::ListOfServices;
        if ((*iter).second->getValue().empty()) {
            for (OptionMap::const_iterator uriiter = (*iter).second->begin(); uriiter != (*iter).second->end(); ++uriiter) {
                insertOneService(*(*uriiter).second, services);
            }
        } else {
            insertOneService(*((*iter).second), services);
        }
        serviceCache->addToCache(Transfer::URIContext((*iter).first), Transfer::ListOfServicesPtr(services));
    }
}

CacheLayer *createNetworkCache(const OptionMap &options) {
    Transfer::CachedServiceLookup *services = new Transfer::CachedServiceLookup;
    Transfer::ServiceManager<Transfer::DownloadHandler> *downServ (
                new Transfer::ServiceManager<Transfer::DownloadHandler>(
                    services,
                    &downloadProtocolRegistry));
    insertServices(options["services"], services);
    return new NetworkCacheLayer(NULL, downServ);
}
void destroyTransferManager(TransferManager*tm) {
    delete tm;
}
TransferManager *initializeTransferManager (const OptionMap& options, GenEventManager *eventMgr) {
    OptionMap &cache = options["cache"];
    CacheLayer *lastCacheLayer = NULL;
    CacheLayer *firstCacheLayer = NULL;
    for (OptionMap::const_iterator iter = cache.begin(); iter != cache.end(); ++iter) {
        CacheLayer *newCacheLayer = CreateCache(*((*iter).second));
        if (lastCacheLayer) {
            lastCacheLayer->setNext(newCacheLayer);
        } else {
            firstCacheLayer = newCacheLayer;
        }
        lastCacheLayer = newCacheLayer;
    }
    Transfer::CachedServiceLookup*downServiceMap,*nameServiceMap,*upServiceMap,*upnameServiceMap;
    Transfer::ServiceManager<Transfer::DownloadHandler> *downServ =
                new Transfer::ServiceManager<Transfer::DownloadHandler>(
                    downServiceMap=new Transfer::CachedServiceLookup,
                    &downloadProtocolRegistry);
            Transfer::ServiceManager<Transfer::NameLookupHandler> *nameServ =
                new Transfer::ServiceManager<Transfer::NameLookupHandler>(
                    nameServiceMap=new Transfer::CachedServiceLookup,
                    &nameProtocolRegistry);
    Transfer::ServiceManager<Transfer::UploadHandler> *uploadServ =
                new Transfer::ServiceManager<Transfer::UploadHandler>(
                    upServiceMap=new Transfer::CachedServiceLookup,
                    &uploadProtocolRegistry);
            Transfer::ServiceManager<Transfer::NameUploadHandler> *upnameServ =
                new Transfer::ServiceManager<Transfer::NameUploadHandler>(
                    upnameServiceMap=new Transfer::CachedServiceLookup,
                    &nameUploadProtocolRegistry);

            insertServices(options["download"], downServiceMap);
            insertServices(options["namelookup"], nameServiceMap);
            insertServices(options["upload"], upServiceMap);
            insertServices(options["nameupload"], upnameServiceMap);
            {
                Transfer::ListOfServices *services = new Transfer::ListOfServices;
                Transfer::ServiceParams params;
                params.set("requesturi","true");
                services->push_back(Transfer::ListOfServices::value_type(
                    Transfer::URIContext("x-shasumofuri:"),
                    params));
                nameServiceMap->addToCache(
                    Transfer::URIContext(),
                    Transfer::ListOfServicesPtr(services));
            }
            {
                Transfer::ListOfServices *services = new Transfer::ListOfServices;
                services->push_back(Transfer::ListOfServices::value_type(
                    Transfer::URIContext(),
                    Transfer::ServiceParams()));
                downServiceMap->addToCache(
                    Transfer::URIContext(),
                    Transfer::ListOfServicesPtr(services));
            }
            {
                Transfer::ListOfServices *services = new Transfer::ListOfServices;
                Transfer::ServiceParams params;
                params.set("requesturi","true");
                services->push_back(Transfer::ListOfServices::value_type(
                    Transfer::URIContext("x-hashed:"),
                    params));
                nameServiceMap->addToCache(
                    Transfer::URIContext("mhash:"),
                    Transfer::ListOfServicesPtr(services));
            }

        return new Transfer::EventTransferManager(
                firstCacheLayer,
                new Transfer::CachedNameLookupManager(nameServ, downServ),
                eventMgr,
				downServ,
                upnameServ,
                uploadServ
            );
}

}
