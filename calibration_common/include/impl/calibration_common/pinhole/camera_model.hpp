/*
 *  Copyright (c) 2013-2014, Filippo Basso <bassofil@dei.unipd.it>
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *     1. Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *     2. Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *     3. Neither the name of the copyright holder(s) nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef IMPL_CALIBRATION_COMMON_PINHOLE_CAMERA_MODEL_HPP_
#define IMPL_CALIBRATION_COMMON_PINHOLE_CAMERA_MODEL_HPP_

#include <calibration_common/pinhole/camera_model.h>
#include <calibration_common/base/opencv_conversion.h>

namespace calibration
{

template <typename Scalar>
  typename Types_<Scalar>::Point3 PinholeCameraModel::projectPixelTo3dRay(const typename Types_<Scalar>::Point2 & pixel_point) const
  {
    assert(initialized());

    typename Types_<Scalar>::Point3 ray;
    ray[0] = Scalar((pixel_point[0] - cx() - Tx()) / fx());
    ray[1] = Scalar((pixel_point[1] - cy() - Ty()) / fy());
    ray[2] = Scalar(1.0);
    ray.normalize();

    return ray;
  }

template <typename Scalar>
  typename Types_<Scalar>::Point2 PinholeCameraModel::project3dToPixel(const typename Types_<Scalar>::Point3 & world_point) const
  {
    assert(initialized());
    assert(P_(2, 3) == 0.0);

    typename Types_<Scalar>::Point2 uv_rect;
    uv_rect[0] = Scalar((fx() * world_point[0] + Tx()) / world_point[2] + cx());
    uv_rect[1] = Scalar((fy() * world_point[1] + Ty()) / world_point[2] + cy());

    return uv_rect;
  }

template <typename Scalar>
  void PinholeCameraModel::projectPixelTo3dRay(const typename Types_<Scalar>::Point2Matrix & pixel_points,
                                               typename Types_<Scalar>::Point3Matrix & world_points) const
  {
    assert(initialized());
    assert(pixel_points.size() == world_points.size());

    Eigen::Array<Scalar, 2, 1> sub(Scalar(-cx() - Tx()), Scalar(-cy() - Ty()));
    Eigen::Array<Scalar, 2, 1> div(Scalar(fx()), Scalar(fy()));

    world_points.matrix().template topRows<2>() = (pixel_points.matrix().array().colwise() + sub).colwise() / div;
    world_points.matrix().template bottomRows<1>().setOnes();
    world_points.matrix().array().rowwise() /= world_points.matrix().colwise().norm().array();
  }

template <typename Scalar>
  typename Types_<Scalar>::Point3Matrix PinholeCameraModel::projectPixelTo3dRay(const typename Types_<Scalar>::Point2Matrix & pixel_points) const
  {
    typename Types_<Scalar>::Point3Matrix world_points(pixel_points.xSize(), pixel_points.ySize());
    projectPixelTo3dRay<Scalar>(pixel_points, world_points);
    return world_points;
  }

template <typename Scalar>
  void PinholeCameraModel::project3dToPixel(const typename Types_<Scalar>::Point3Matrix & world_points,
                                            typename Types_<Scalar>::Point2Matrix & pixel_points) const
  {
    assert(initialized());
    assert(P_(2, 3) == 0.0);
    assert(pixel_points.size() == world_points.size());

    Eigen::Array<double, 2, 1> prod(fx(), fy());
    Types_<double>::Point2 sum(Tx(), Ty());
    Types_<double>::Point2 sum_final(cx(), cy());

    pixel_points.matrix() = world_points.matrix().template topRows<2>();
    pixel_points.matrix().array().colwise() *= prod.template cast<Scalar>();
    pixel_points.matrix().colwise() += sum.template cast<Scalar>();
    pixel_points.matrix().array().rowwise() /= world_points.matrix().array().template bottomRows<1>();
    pixel_points.matrix().colwise() += sum_final.template cast<Scalar>();
  }

template <typename Scalar>
  typename Types_<Scalar>::Point2Matrix PinholeCameraModel::project3dToPixel(const typename Types_<Scalar>::Point3Matrix & world_points) const
  {
    typename Types_<Scalar>::Point2Matrix pixel_points(world_points.xSize(), world_points.ySize());
    project3dToPixel<Scalar>(world_points, pixel_points);
    return pixel_points;
  }

template <typename Scalar>
  typename Types_<Scalar>::Pose PinholeCameraModel::estimatePose(const typename Types_<Scalar>::Point2Matrix & points_image,
                                                                 const typename Types_<Scalar>::Point3Matrix & points_object) const
  {
    assert(points_image.size() == points_object.size());

    cv::Mat_<cv::Vec<Scalar, 2> > cv_points_image;
    cv::Mat_<cv::Vec<Scalar, 3> > cv_points_object;
    OpenCVConversion<Scalar>::toOpenCV(points_image.matrix(), cv_points_image);
    OpenCVConversion<Scalar>::toOpenCV(points_object.matrix(), cv_points_object);

    cv::Vec<Scalar, 3> cv_r, cv_t;
    cv::solvePnP(cv_points_object, cv_points_image, intrinsicMatrix(), distortionCoeffs(), cv_r, cv_t);

    Types::Vector3 r;
    r << cv_r[0], cv_r[1], cv_r[2];
    Types::Vector3 t;
    t << cv_t[0], cv_t[1], cv_t[2];

    Types::AngleAxis rotation(r.norm(), r.normalized());
    Types::Translation3 translation(t);

    return translation * rotation;
  }

} /* namespace calibration */

#endif /* IMPL_CALIBRATION_COMMON_PINHOLE_CAMERA_MODEL_HPP_ */
