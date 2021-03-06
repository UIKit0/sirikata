// Copyright (c) 2011 Sirikata Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef _SIRIKATA_MESH_BILLBOARD_HPP_
#define _SIRIKATA_MESH_BILLBOARD_HPP_

#include <sirikata/mesh/Platform.hpp>
#include <sirikata/mesh/Visual.hpp>

namespace Sirikata {
namespace Mesh {

struct SIRIKATA_MESH_EXPORT Billboard : public Visual {
  private:
    static String sType;

  public:
    Billboard();
    Billboard(const String& img);
    virtual ~Billboard();

    virtual const String& type() const;

    String image;
    float32 aspectRatio; // Width / height
    enum BillboardFacing {
        FACING_CAMERA,
        FACING_FIXED
    };
    BillboardFacing facing;
};

typedef std::tr1::shared_ptr<Billboard> BillboardPtr;
typedef std::tr1::weak_ptr<Billboard> BillboardWPtr;


} // namespace Mesh
} // namespace Sirikata

#endif //_SIRIKATA_MESH_BILLBOARD_HPP_
