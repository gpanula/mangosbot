/*
 * Copyright (C) 2005-2012 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MANGOSSERVER_MOVEPLINE_H
#define MANGOSSERVER_MOVEPLINE_H

#include "spline.h"
#include "MoveSplineInitArgs.h"
#include "WorldLocation.h"

namespace Movement
{
    // MoveSpline represents smooth catmullrom or linear curve and point that moves belong it
    // curve can be cyclic - in this case movement will be cyclic
    // point can have vertical acceleration motion componemt(used in fall, parabolic movement)
    class MoveSpline
    {
        public:
            typedef Spline<int32> MySpline;
            enum UpdateResult
            {
                Result_None         = 0x01,
                Result_Arrived      = 0x02,
                Result_NextCycle    = 0x04,
                Result_NextSegment  = 0x08,
            };
            friend class PacketBuilder;
        protected:
            MySpline        spline;

            FacingInfo      facing;

            uint32          m_Id;

            MoveSplineFlag  splineflags;

            int32           time_passed;
            // currently duration mods are unused, but its _currently_
            // float           duration_mod;
            // float           duration_mod_next;
            float           vertical_acceleration;
            float           initialOrientation;
            int32           effect_start_time;
            int32           point_Idx;
            int32           point_Idx_offset;

            void init_spline(const MoveSplineInitArgs& args);
        protected:

            const MySpline::ControlArray& getPath() const { return spline.getPoints();}
            void computeParabolicElevation(float& el) const;
            void computeFallElevation(float& el) const;

            UpdateResult _updateState(int32& ms_time_diff);
            int32 next_timestamp() const { return spline.length(point_Idx + 1);}
            int32 segment_time_elapsed() const { return next_timestamp() - time_passed;}
            int32 timeElapsed() const { return Duration() - time_passed;}
            int32 timePassed() const { return time_passed;}

        public:
            const MySpline& _Spline() const { return spline;}
            int32 _currentSplineIdx() const { return point_Idx;}
            void _Finalize();
            void _Interrupt() { splineflags.done = true;}

        public:

            void Initialize(const MoveSplineInitArgs&);
            bool Initialized() const { return !spline.empty();}

            explicit MoveSpline();

            template<class UpdateHandler>
            void updateState(int32 difftime, UpdateHandler& handler)
            {
                MANGOS_ASSERT(Initialized());
                do
                    handler(_updateState(difftime));
                while (difftime > 0);
            }

            void updateState(int32 difftime)
            {
                MANGOS_ASSERT(Initialized());
                do _updateState(difftime);
                while (difftime > 0);
            }

            Location ComputePosition() const;

            uint32 GetId() const { return m_Id;}
            bool Finalized() const { return splineflags.done; }
            bool isCyclic() const { return splineflags.cyclic;}
            const Vector3 FinalDestination() const { return Initialized() ? spline.getPoint(spline.last()) : Vector3();}
            const Vector3 CurrentDestination() const { return Initialized() ? spline.getPoint(point_Idx + 1) : Vector3();}
            int32 currentPathIdx() const;

            int32 Duration() const { return spline.length();}

            std::string ToString() const;
    };
}
#endif // MANGOSSERVER_MOVEPLINE_H