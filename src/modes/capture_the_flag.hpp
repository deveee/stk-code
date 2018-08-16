//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef CAPTURE_THE_FLAG_HPP
#define CAPTURE_THE_FLAG_HPP

#include "modes/free_for_all.hpp"

#include <vector>
#include <string>

namespace irr
{
    namespace scene
    {
        class IAnimatedMeshSceneNode; class IAnimatedMesh;
    }
}

class CaptureTheFlag : public FreeForAll
{
private:
    scene::IAnimatedMeshSceneNode* m_red_flag_node;

    scene::IAnimatedMeshSceneNode* m_blue_flag_node;

    irr::scene::IAnimatedMesh* m_red_flag_mesh;

    irr::scene::IAnimatedMesh* m_blue_flag_mesh;

public:
    // ------------------------------------------------------------------------
    CaptureTheFlag();
    // ------------------------------------------------------------------------
    virtual ~CaptureTheFlag();
    // ------------------------------------------------------------------------
    virtual void init() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual bool hasTeam() const OVERRIDE                      { return true; }
    // ------------------------------------------------------------------------

};   // CaptureTheFlag

#endif
