/*
 *  Copyright (C) 2013 - Filippo Basso <bassofil@dei.unipd.it>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CALIBRATION_COMMON_OBJECTS_PLANAR_OBJECT_H_
#define CALIBRATION_COMMON_OBJECTS_PLANAR_OBJECT_H_

#include <calibration_common/objects/base_object.h>
#include <calibration_common/depth/view.h>

namespace calibration
{

class PlanarObject : public BaseObject
{
public:

  typedef boost::shared_ptr<PlanarObject> Ptr;
  typedef boost::shared_ptr<const PlanarObject> ConstPtr;

  explicit PlanarObject()
    : BaseObject(),
      plane_(PLANE_XY)
  {
    // Do nothing
  }

  virtual ~PlanarObject()
  {
    // Do nothing
  }

  virtual void transform(const Types::Transform & transform)
  {
    plane_.transform(transform);
    BaseObject::transform(transform);
  }

  const Types::Plane & plane() const
  {
    return plane_;
  }

  void setPlane(const Types::Plane & plane)
  {
    BaseObject::transform(Util::plane3dTransform(plane_, plane)); // TODO make sense???
    plane_ = plane;
  }

  void toMarker(visualization_msgs::Marker & marker) const;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:

  Types::Plane plane_;

};

//template <typename Derived>
//PlanarObject::PlanarObject(const ColorView<Derived> & view)
//  : BaseObject(view), plane_(view.object()->plane())
//{
//  plane_.transform(pose());
//}

//template <typename PointT>
//  PlanarObject::PlanarObject(const DepthView<PlanarObject, PointT> & view)
//    : BaseObject("", view.sensor())
//  {
//    std::stringstream ss;
//    ss << view.object()->frameId() << "_" << view.id();
//    BaseObject::frame_id_ = ss.str();
//
//    plane_ = fitPlane(view.interestPoints());
//    BaseObject::transform(Util::plane3dTransform(view.object()->plane(), plane_));
//  }

} /* namespace calibration */
#endif /* CALIBRATION_COMMON_OBJECTS_PLANAR_OBJECT_H_ */
