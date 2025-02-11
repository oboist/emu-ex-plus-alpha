/*  This file is part of Imagine.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Imagine.  If not, see <http://www.gnu.org/licenses/> */

#include <imagine/gfx/Mat4.hh>
#include <imagine/glm/geometric.hpp>
#include <imagine/glm/gtc/matrix_transform.hpp>
#include <imagine/glm/gtc/matrix_inverse.hpp>

namespace glm
{
	template <typename T, typename U, precision P>
	GLM_FUNC_QUALIFIER tvec3<T, P> unProjectWithInverse
	(
		tvec3<T, P> const & win,
		tmat4x4<T, P> const & inverse,
		tvec4<U, P> const & viewport
	)
	{
		tvec4<T, P> tmp = tvec4<T, P>(win, T(1));
		tmp.x = (tmp.x - T(viewport[0])) / T(viewport[2]);
		tmp.y = (tmp.y - T(viewport[1])) / T(viewport[3]);
		tmp = tmp * T(2) - T(1);

		tvec4<T, P> obj = inverse * tmp;
		obj /= obj.w;

		return tvec3<T, P>(obj);
	}
}

namespace IG::Gfx
{

Mat4 Mat4::translate(Vec3 translation) const
{
	return glm::translate(*this, translation);
}

Mat4 Mat4::makeTranslate(Vec3 translation)
{
	return glm::translate(glm::identity<glm::mat4>(), translation);
}

Mat4 Mat4::makePerspectiveFovRH(float fovy, float aspect, float znear, float zfar)
{
	return glm::perspective(fovy, aspect, znear, zfar);
}

Mat4 Mat4::scale(Vec3 factors) const
{
	return glm::scale(*this, factors);
}

Mat4 Mat4::rotate(float angle, Vec3 axis) const
{
	return glm::rotate(*this, angle, axis);
}

Mat4 Mat4::invert() const
{
	return glm::inverse(*this);
}

Vec3 Mat4::project(Rect2<int> viewport, Vec3 obj) const
{
	glm::ivec4 viewportVec {viewport.x, viewport.y, viewport.x2, viewport.y2};
	return glm::project(obj, {}, *this, viewportVec);
}

Vec3 Mat4::unproject(Rect2<int> viewport, Vec3 win, Mat4 inverse) const
{
	glm::ivec4 viewportVec {viewport.x, viewport.y, viewport.x2, viewport.y2};
	return glm::unProjectWithInverse(win, inverse, viewportVec);
}

}
