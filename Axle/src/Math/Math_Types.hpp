#pragma once

#include "axpch.hpp"
#include "Core/Types.hpp"

#include "Core/Core.hpp"
#include "Mathf.hpp"

namespace Axle {
	/// Representation of 2D vectors and points
	struct Vector2 {
		union {
			f32 x, r, s, u;
		};
		union {
			f32 y, g, t, v;
		};

		Vector2(f32 x, f32 y) : x(x), y(y) {}
		Vector2() : x(0.0f), y(0.0f) {}

		/// Shorthand of writing Vector2(0.0f, 0.0f)
		inline static Vector2 Zero() { return Vector2(0.0f, 0.0f); }
		/// Shorthand of writing Vector2(1.0f, 1.0f)
		inline static Vector2 One() { return Vector2(1.0f, 1.0f); }
		/// Shorthand of writing Vector2(1.0f, 0.0f)
		inline static Vector2 Right() { return Vector2(1.0f, 0.0f); }
		/// Shorthand of writing Vector2(-1.0f, 0.0f)
		inline static Vector2 Left() { return Vector2(-1.0f, 0.0f); }
		/// Shorthand of writing Vector2(0.0f, 1.0f)
		inline static Vector2 Up() { return Vector2(0.0f, 1.0f); }
		/// Shorthand of writing Vector2(0.0f, -1.0f)
		inline static Vector2 Down() { return Vector2(0.0f, -1.0f); }

		/**
		 * Returns the squared magnitude of the vector
		 *
		 * It's helpful if you don't need to compare the magnitudes directly because doing the square root is computer
		 * intensive.
		 *
		 * @returns The squared magnitude of the vector
		 */
		inline f32 SqrMagnitude() const { return x * x + y * y; }

		/**
		 * Returns the magnitude of the vector
		 *
		 * If you only need to compare magnitudes of some vectors, you can use SsqrMagnitude (computing squared
		 * magnitudes is faster)
		 *
		 * @returns The magnitude of the vector
		 */
		inline f32 Magnitude() const { return Mathf::Sqrt(SqrMagnitude()); }

		/// Modifies the current vector to have a magnitude of 1
		// TODO: Check for division by zero
		inline void Normalize() {
			const f32 magnitude = Magnitude();
			this->x /= magnitude;
			this->y /= magnitude;
		}

		/**
		 * Returns a new vector with the same direction as the current one, but with a magnitude of 1
		 *
		 * @returns A new vector with the same direction but with a magnitude of 1
		 */
		inline Vector2 Normalized() const {
			Vector2 vec = Vector2(this->x, this->y);
			vec.Normalize();
			return vec;
		}

		/**
		 * Returns a new vector that is perpendicular to the current vector
		 *
		 * The result is always rotated 90-degrees in a counter-clockwise direction for a 2D coordinate system where the
		 * positive Y axis goes up.
		 *
		 * @returns A new vector that is perpendicular to the current vector.
		 */
		inline Vector2 Perpendicular() const { return Vector2(-(this->y), this->x); }

		/**
		 * Calculates the euclidean distance between two vectors
		 *
		 * @param a The first vector
		 * @param b The second vector
		 *
		 * @returns The distance between two vectors
		 */
		inline static f32 Distance(const Vector2& a, const Vector2& b) { return (a - b).Magnitude(); }

		/**
		 * Calculates the dot product of two vectors
		 *
		 * @param a The first vector
		 * @param b The second vector
		 *
		 * @returns The dot product of the two vectors
		 */
		inline static f32 Dot(const Vector2& a, const Vector2& b) { return a.x * b.x + a.y * b.y; }

		/**
		 * Calculates the angle in radians between two vectors
		 *
		 * @param a The first vector
		 * @param b The second vector
		 *
		 * @returns The angle in radians between two vectors
		 */
		inline static f32 Angle(const Vector2& a, const Vector2& b) {
			return Mathf::Acos(Vector2::Dot(a, b) / (a.Magnitude() * b.Magnitude()));
		}

		/**
		 * Performs a linear interpolation between two vectors without clamping the interpolation factor.
		 * If you want the interpolator factor to be clamped use Lerp instead.
		 *
		 * @param a The origin vector
		 * @param b The destination vector
		 * @param t The interpolation factor
		 *
		 * @returns The interpolated vector
		 */
		inline static Vector2 LerpUnclamped(const Vector2& a, const Vector2& b, f32 t) {
			return a * (1.0f - t) + b * t;
		}

		/**
		 * Performs a linear interpolation between two vectors without clamping the interpolation factor.
		 *
		 * @param a The origin vector
		 * @param b The destination vector
		 * @param t The interpolation factor
		 *
		 * @returns The interpolated vector
		 */
		inline static Vector2 Lerp(const Vector2& a, const Vector2& b, f32 t) {
			return LerpUnclamped(a, b, Mathf::Clamp01(t));
		}

		/**
		 * Reflects a vector off the surface defined by a normal.
		 *
		 * Note: inNormal does not need to be normalized.
		 *
		 * @param inDirection The incident vector
		 * @param inNormal The surface's normal vector
		 *
		 * @returns The reflected vector
		 */
		inline static Vector2 Reflect(const Vector2& inDirection, const Vector2& inNormal) {
			Vector2 inNormalNormalized = inNormal.Normalized();
			return inDirection - inNormalNormalized * 2 * Vector2::Dot(inDirection, inNormalNormalized);
		}

		/**
		 * Calculates the scalar projection of a vector onto another vector
		 *
		 * @param toProject The vector to be projected
		 * @param onProject The vector to project onto
		 *
		 * @returns The scalar projection of toProject onto onProject.
		 */
		inline static f32 ScalarProjection(const Vector2& toProject, const Vector2& onProject) {
			return Vector2::Dot(toProject, onProject) / onProject.Magnitude();
		}

		/**
		 * Projects a vector onto another vector
		 *
		 * @param toProject The vector to be projected
		 * @param onProject The vector to project onto
		 *
		 * @returns The orthogonal projection of toProject onto onProject
		 */
		inline static Vector2 Project(const Vector2& toProject, const Vector2& onProject) {
			return onProject.Normalized() * ScalarProjection(toProject, onProject);
		}

		Vector2 operator+(const Vector2& other) const { return Vector2(x + other.x, y + other.y); }
		Vector2 operator-(const Vector2& other) const { return Vector2(x - other.x, y - other.y); }
		Vector2 operator*(const f32& other) const { return Vector2(x * other, y * other); }
		Vector2 operator/(const f32& other) const { return Vector2(x / other, y / other); }
		bool operator==(const Vector2& other) const {
			return Mathf::Approximately(x, other.x) && Mathf::Approximately(y, other.y);
		}
	};

	/// Representation of 3D vectors and points
	struct Vector3 {
		union {
			f32 x, r, s, u;
		};
		union {
			f32 y, g, t, v;
		};
		union {
			f32 z, b, p, w;
		};

		Vector3(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {}
		Vector3() : x(0.0f), y(0.0f), z(0.0f) {}

		/// Shorthand of writing Vector3(0.0f, 0.0f, 0.0f)
		inline static Vector3 Zero() { return Vector3(0.0f, 0.0f, 0.0f); }
		/// Shorthand of writing Vector3(1.0f, 1.0f, 1.0f)
		inline static Vector3 One() { return Vector3(1.0f, 1.0f, 1.0f); }
		/// Shorthand of writing Vector3(1.0f, 0.0f, 0.0f)
		inline static Vector3 Right() { return Vector3(1.0f, 0.0f, 0.0f); }
		/// Shorthand of writing Vector3(-1.0f, 0.0f, 0.0f)
		inline static Vector3 Left() { return Vector3(-1.0f, 0.0f, 0.0f); }
		/// Shorthand of writing Vector3(0.0f, 1.0f, 0.0f)
		inline static Vector3 Up() { return Vector3(0.0f, 1.0f, 0.0f); }
		/// Shorthand of writing Vector3(0.0f, -1.0f, 0.0f)
		inline static Vector3 Down() { return Vector3(0.0f, -1.0f, 0.0f); }
		/// Shorthand of writing Vector3(0.0f, 0.0f, 1.0f)
		inline static Vector3 Forward() { return Vector3(0.0f, 0.0f, 1.0f); }
		/// Shorthand of writing Vector3(0.0f, 0.0f, -1.0f)
		inline static Vector3 Back() { return Vector3(0.0f, 0.0f, -1.0f); }

		/**
		 * Returns the squared magnitude of the vector
		 *
		 * It's helpful if you don't need to compare the magnitudes directly because doing the square root is computer
		 * intensive.
		 *
		 * @returns The squared magnitude of the vector
		 */
		inline f32 SqrMagnitude() const { return x * x + y * y + z * z; }

		/**
		 * Returns the magnitude of the vector
		 *
		 * If you only need to compare magnitudes of some vectors, you can use SsqrMagnitude (computing squared
		 * magnitudes is faster)
		 *
		 * @returns The magnitude of the vector
		 */
		inline f32 Magnitude() const { return Mathf::Sqrt(SqrMagnitude()); }

		/// Modifies the current vector to have a magnitude of 1
		inline void Normalize() {
			// TODO: Check for division by zero
			const f32 magnitude = Magnitude();
			this->x /= magnitude;
			this->y /= magnitude;
			this->z /= magnitude;
		}

		/**
		 * Returns a new vector with the same direction as the current one, but with a magnitude of 1
		 *
		 * @returns A new vector with the same direction but with a magnitude of 1
		 */
		inline Vector3 Normalized() const {
			Vector3 vec = Vector3(this->x, this->y, this->z);
			vec.Normalize();
			return vec;
		}

		/**
		 * Calculates the euclidean distance between two vectors
		 *
		 * @param a The first vector
		 * @param b The second vector
		 *
		 * @returns The distance between two vectors
		 */
		inline static f32 Distance(const Vector3& a, const Vector3& b) { return (a - b).Magnitude(); }

		/**
		 * Calculates the dot product of two vectors
		 *
		 * @param a The first vector
		 * @param b The second vector
		 *
		 * @returns The dot product of the two vectors
		 */
		inline static f32 Dot(const Vector3& a, const Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

		/**
		 * Calculates the angle in radians between two vectors
		 *
		 * @param a The first vector
		 * @param b The second vector
		 *
		 * @returns The angle in radians between two vectors
		 */
		inline static f32 Angle(const Vector3& a, const Vector3& b) {
			return Mathf::Acos(Vector3::Dot(a, b) / (a.Magnitude() * b.Magnitude()));
		}

		/**
		 * Performs a linear interpolation between two vectors without clamping the interpolation factor.
		 * If you want the interpolator factor to be clamped use Lerp instead.
		 *
		 * @param a The origin vector
		 * @param b The destination vector
		 * @param t The interpolation factor
		 *
		 * @returns The interpolated vector
		 */
		inline static Vector3 LerpUnclamped(const Vector3& a, const Vector3& b, const f32 t) {
			return a * (1.0f - t) + b * t;
		}

		/**
		 * Performs a linear interpolation between two vectors without clamping the interpolation factor.
		 *
		 * @param a The origin vector
		 * @param b The destination vector
		 * @param t The interpolation factor
		 *
		 * @returns The interpolated vector
		 */
		inline static Vector3 Lerp(const Vector3& a, const Vector3& b, const f32 t) {
			return LerpUnclamped(a, b, Mathf::Clamp01(t));
		}

		/**
		 * Reflects a vector off the surface defined by a normal
		 *
		 * Note: inNormal does not need to be normalized.
		 *
		 * @param inDirection The incident vector
		 * @param inNormal The surface's normal vector
		 *
		 * @returns The reflected vector
		 */
		inline static Vector3 Reflect(const Vector3& inDirection, const Vector3& inNormal) {
			Vector3 inNormalNormalized = inNormal.Normalized();
			return inDirection - inNormalNormalized * 2 * Vector3::Dot(inDirection, inNormalNormalized);
		}

		/**
		 * Calculates the scalar projection of a vector onto another vector
		 *
		 * @param toProject The vector to be projected
		 * @param onProject The vector to project onto
		 *
		 * @returns The scalar projection of toProject onto onProject.
		 */
		inline static f32 ScalarProjection(const Vector3& toProject, const Vector3& onProject) {
			return Vector3::Dot(toProject, onProject) / onProject.Magnitude();
		}

		/**
		 * Projects a vector onto another vector
		 *
		 * @param toProject The vector to be projected
		 * @param onProject The vector to project onto
		 *
		 * @returns The orthogonal projection of toProject onto onProject
		 */
		inline static Vector3 Project(const Vector3& toProject, const Vector3& onProject) {
			return onProject.Normalized() * ScalarProjection(toProject, onProject);
		}

		/**
		 * Calculates the cross product of two vectors
		 *
		 * @param a The first vector
		 * @param b The second vector
		 *
		 * @returns The cross product of two vectors
		 */
		inline static Vector3 Cross(const Vector3& a, const Vector3& b) {
			return Vector3(a.y * b.z - b.y * a.z, b.x * a.z - a.x * b.z, a.x * b.y - b.x * a.y);
		}

		/**
		 * Projects a vector onto a plane.
		 *
		 * For a given plane described by planeNormal and a given vector vector, the method generates a new vector
		 * orthogonal to planeNormal and parallel to the plane. Note: planeNormal does not need to be normalized
		 *
		 * @param vector The vector to be projected
		 * @param planeNormal The normal which defines the plane
		 *
		 * @returns The orthogonal projection of vector on the plane
		 */
		inline static Vector3 ProjectOnPlane(const Vector3& vector, const Vector3& planeNormal) {
			Vector3 projectionOnNormal = Project(vector, planeNormal);
			return vector - projectionOnNormal;
		}

		Vector3 operator+(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
		Vector3 operator-(const Vector3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
		Vector3 operator*(const f32& other) const { return Vector3(x * other, y * other, z * other); }
		Vector3 operator/(const f32& other) const { return Vector3(x / other, y / other, z / other); }
		bool operator==(const Vector3& other) const {
			return Mathf::Approximately(x, other.x) && Mathf::Approximately(y, other.y) &&
				Mathf::Approximately(z, other.z);
		}
	};

	/// Representation of 4D vectors and points
	struct Vector4 {
		union {
			f32 x, r, s;
		};
		union {
			f32 y, g, t;
		};
		union {
			f32 z, b, p;
		};
		union {
			f32 w, a, q;
		};

		Vector4(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}
		Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}

		/// Shorthand of writing Vector4(0.0f, 0.0f, 0.0f, 0.0f)
		inline static Vector4 Zero() { return Vector4(0.0f, 0.0f, 0.0f, 0.0f); }

		/// Shorthand of writing Vector4(1.0f, 1.0f, 1.0f, 1.0f)
		inline static Vector4 One() { return Vector4(1.0f, 1.0f, 1.0f, 1.0f); }

		/**
		 * Returns the squared magnitude of the vector
		 *
		 * It's helpful if you don't need to compare the magnitudes directly because doing the square root is computer
		 * intensive.
		 *
		 * @returns The squared magnitude of the vector
		 */
		inline f32 SqrMagnitude() { return x * x + y * y + z * z + w * w; }

		/**
		 * Returns the magnitude of the vector
		 *
		 * If you only need to compare magnitudes of some vectors, you can use SsqrMagnitude (computing squared
		 * magnitudes is faster)
		 *
		 * @returns The magnitude of the vector
		 */
		inline f32 Magnitude() { return Mathf::Sqrt(SqrMagnitude()); }

		/// Modifies the current vector to have a magnitude of 1
		inline void Normalize() {
			const f32 magnitude = Magnitude();
			this->x /= magnitude;
			this->y /= magnitude;
			this->z /= magnitude;
			this->w /= magnitude;
		}

		/**
		 * Returns a new vector with the same direction as the current one, but with a magnitude of 1
		 *
		 * @returns A new vector with the same direction but with a magnitude of 1
		 */
		inline Vector4 Normalized() {
			Vector4 vec = Vector4(this->x, this->y, this->z, this->w);
			vec.Normalize();
			return vec;
		}

		/**
		 * Converts the current vector4 to a vector3
		 *
		 * @returns The converted Vector3
		 */
		inline Vector3 ConvertToVector3() { return Vector3(this->x, this->y, this->z); }

		/**
		 * Converts the current vector4 to a vector2
		 *
		 * @returns The converted Vector2
		 */
		inline Vector2 ConvertToVector2() { return Vector2(this->x, this->y); }

		/**
		 * Calculates the dot product of two vectors
		 *
		 * @param a The first vector
		 * @param b The second vector
		 *
		 * @returns The dot product of the two vectors
		 */
		inline static f32 Dot(Vector4& a, Vector4& b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

		Vector4 operator+(const Vector4& other) const {
			return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
		}
		Vector4 operator-(const Vector4& other) const {
			return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
		}
		Vector4 operator*(const f32& other) const { return Vector4(x * other, y * other, z * other, w * other); }
		Vector4 operator/(const f32& other) const { return Vector4(x / other, y / other, z / other, w / other); }
		bool operator==(const Vector4& other) const {
			return Mathf::Approximately(x, other.x) && Mathf::Approximately(y, other.y) &&
				Mathf::Approximately(z, other.z) && Mathf::Approximately(w, other.w);
		}
	};

	struct Matrix4x4 {
		f32 data[16] = {};

		Matrix4x4() = default;

		/**
		 * Returns the Identity matrix
		 *
		 * @returns The Identity matrix
		 */
		inline static Matrix4x4 Indentity() {
			Matrix4x4 mat;
			mat.data[0] = 1.0f;
			mat.data[5] = 1.0f;
			mat.data[10] = 1.0f;
			mat.data[15] = 1.0f;
			return mat;
		}

		/**
		 * Shorthand for creating a matrix full of zeros
		 *
		 * @returns A matrix full of zeros
		 */
		inline static Matrix4x4 Zero() {
			Matrix4x4 mat;
			return mat;
		}

		/**
		 * Creates and returns an orthographic projection matrix. Typically used to
		 * render flat or 2D scenes.
		 *
		 * @param left The left side of the view frustum.
		 * @param right The right side of the view frustum.
		 * @param bottom The bottom side of the view frustum.
		 * @param top The top side of the view frustum.
		 * @param nearClip The near clipping plane distance.
		 * @param farClip The far clipping plane distance.
		 *
		 * @returns A new orthographic projection matrix.
		 */
		inline static Matrix4x4 OrthographicProjection(f32 left, f32 right, f32 bottom, f32 top, f32 nearClip, f32 farClip) {
			Matrix4x4 mat;
			mat.data[0] = 2.0f / (right - left);
			mat.data[5] = 2.0f / (top - bottom);
			mat.data[10] = -2.0f / (farClip - nearClip);

			mat.data[12] = -(right + left) / (right - left);
			mat.data[13] = -(top + bottom) / (top - bottom);
			mat.data[14] = -(farClip + nearClip) / (farClip - nearClip);
			return mat;
		}

		/**
		 * Creates and returns a perspective matrix. Typically used to render 3d scenes.
		 *
		 * @param fovRadians The field of view in radians.
		 * @param aspectRatio The aspect ratio.
		 * @param nearClip The near clipping plane distance.
		 * @param farClip The far clipping plane distance.
		 *
		 * @returns A new perspective matrix.
		 */
		inline static Matrix4x4 PerspectiveProjection(f32 fovRadians, f32 aspectRatio, f32 nearClip, f32 farClip) {
			Matrix4x4 mat;
			f32 f = 1.0f / Mathf::Tan(fovRadians / 2.0f);

			mat.data[0] = f / aspectRatio;
			mat.data[5] = f;
			mat.data[10] = (farClip + nearClip) / (nearClip - farClip);
			mat.data[11] = -1.0f;
			mat.data[14] = (2.0f * farClip * nearClip) / (nearClip - farClip);

			return mat;
		}

		/**
		 * Creates and returns a look-at matrix, or a matrix looking
		 * at target from the perspective of position.
		 *
		 * @param position The position of the matrix.
		 * @param target The position to "look at".
		 * @param up The up vector.
		 *
		 * @returns A matrix looking at target from the perspective of position.
		 */
		inline static Matrix4x4 LookAt(Vector3 position, Vector3 target, Vector3 up) {
			Vector3 zAxis = (position - target).Normalized();
			Vector3 xAxis = Vector3::Cross(up, zAxis).Normalized();
			Vector3 yAxis = Vector3::Cross(zAxis, xAxis).Normalized();

			Matrix4x4 mat;

			mat.data[0] = xAxis.x;
			mat.data[1] = yAxis.x;
			mat.data[2] = zAxis.x;
			mat.data[3] = 0.0f;
			mat.data[4] = xAxis.y;
			mat.data[5] = yAxis.y;
			mat.data[6] = zAxis.y;
			mat.data[7] = 0.0f;
			mat.data[8] = xAxis.z;
			mat.data[9] = yAxis.z;
			mat.data[10] = zAxis.z;
			mat.data[11] = 0.0f;
			mat.data[12] = -Vector3::Dot(xAxis, position);
			mat.data[13] = -Vector3::Dot(yAxis, position);
			mat.data[14] = -Vector3::Dot(zAxis, position);
			mat.data[15] = 1.0f;

			return mat;
		}

		/**
		 * Returns a transposed copy of the provided matrix (rows->colums)
		 *
		 * @param matrix The matrix to be transposed.
		 *
		 * @returns A transposed copy of of the provided matrix.
		 */
		inline Matrix4x4 Transpose() {
			Matrix4x4 out_matrix;

			for (i32 i = 0; i < 4; i++) {
				for (i32 j = 0; j < 4; j++) {
					out_matrix.data[i * 4 + j] = data[j * 4 + i];
				}
			}

			return out_matrix;
		}

		/**
		 * Creates and returns an inverse of the provided matrix.
		 *
		 * @param matrix The matrix to be inverted.
		 *
		 * @returns A inverted copy of the provided matrix.
		 */
		inline Matrix4x4 Inverse() {
			const f32* m = data;

			f32 t0 = m[10] * m[15];
			f32 t1 = m[14] * m[11];
			f32 t2 = m[6] * m[15];
			f32 t3 = m[14] * m[7];
			f32 t4 = m[6] * m[11];
			f32 t5 = m[10] * m[7];
			f32 t6 = m[2] * m[15];
			f32 t7 = m[14] * m[3];
			f32 t8 = m[2] * m[11];
			f32 t9 = m[10] * m[3];
			f32 t10 = m[2] * m[7];
			f32 t11 = m[6] * m[3];
			f32 t12 = m[8] * m[13];
			f32 t13 = m[12] * m[9];
			f32 t14 = m[4] * m[13];
			f32 t15 = m[12] * m[5];
			f32 t16 = m[4] * m[9];
			f32 t17 = m[8] * m[5];
			f32 t18 = m[0] * m[13];
			f32 t19 = m[12] * m[1];
			f32 t20 = m[0] * m[9];
			f32 t21 = m[8] * m[1];
			f32 t22 = m[0] * m[5];
			f32 t23 = m[4] * m[1];

			Matrix4x4 out_matrix;
			f32* o = out_matrix.data;

			o[0] = (t0 * m[5] + t3 * m[9] + t4 * m[13]) - (t1 * m[5] + t2 * m[9] + t5 * m[13]);
			o[1] = (t1 * m[1] + t6 * m[9] + t9 * m[13]) - (t0 * m[1] + t7 * m[9] + t8 * m[13]);
			o[2] = (t2 * m[1] + t7 * m[5] + t10 * m[13]) - (t3 * m[1] + t6 * m[5] + t11 * m[13]);
			o[3] = (t5 * m[1] + t8 * m[5] + t11 * m[9]) - (t4 * m[1] + t9 * m[5] + t10 * m[9]);

			f32 d = 1.0f / (m[0] * o[0] + m[4] * o[1] + m[8] * o[2] + m[12] * o[3]);

			o[0] = d * o[0];
			o[1] = d * o[1];
			o[2] = d * o[2];
			o[3] = d * o[3];
			o[4] = d * ((t1 * m[4] + t2 * m[8] + t5 * m[12]) - (t0 * m[4] + t3 * m[8] + t4 * m[12]));
			o[5] = d * ((t0 * m[0] + t7 * m[8] + t8 * m[12]) - (t1 * m[0] + t6 * m[8] + t9 * m[12]));
			o[6] = d * ((t3 * m[0] + t6 * m[4] + t11 * m[12]) - (t2 * m[0] + t7 * m[4] + t10 * m[12]));
			o[7] = d * ((t4 * m[0] + t9 * m[4] + t10 * m[8]) - (t5 * m[0] + t8 * m[4] + t11 * m[8]));
			o[8] = d * ((t12 * m[7] + t15 * m[11] + t16 * m[15]) - (t13 * m[7] + t14 * m[11] + t17 * m[15]));
			o[9] = d * ((t13 * m[3] + t18 * m[11] + t21 * m[15]) - (t12 * m[3] + t19 * m[11] + t20 * m[15]));
			o[10] = d * ((t14 * m[3] + t19 * m[7] + t22 * m[15]) - (t15 * m[3] + t18 * m[7] + t23 * m[15]));
			o[11] = d * ((t17 * m[3] + t20 * m[7] + t23 * m[11]) - (t16 * m[3] + t21 * m[7] + t22 * m[11]));
			o[12] = d * ((t14 * m[10] + t17 * m[14] + t13 * m[6]) - (t16 * m[14] + t12 * m[6] + t15 * m[10]));
			o[13] = d * ((t20 * m[14] + t12 * m[2] + t19 * m[10]) - (t18 * m[10] + t21 * m[14] + t13 * m[2]));
			o[14] = d * ((t18 * m[6] + t23 * m[14] + t15 * m[2]) - (t22 * m[14] + t14 * m[2] + t19 * m[6]));
			o[15] = d * ((t22 * m[10] + t16 * m[2] + t21 * m[6]) - (t20 * m[6] + t23 * m[10] + t17 * m[2]));

			return out_matrix;
		}

		/**
		 * Creates a translation matrix
		 *
		 * @param position The position of the translation
		 *
		 * @returns A translation matrix
		 */
		static Matrix4x4 Translation(Vector3 position) {
			Matrix4x4 mat = Matrix4x4::Indentity();
			mat.data[12] = position.x;
			mat.data[13] = position.y;
			mat.data[14] = position.z;
			return mat;
		}

		/**
		 * Returns a scale matrix using the provided scale.
		 *
		 * @param scale The 3-component scale.
		 *
		 * @returns A scale matrix.
		 */
		static Matrix4x4 Scale(Vector3 scale) {
			Matrix4x4 mat = Matrix4x4::Indentity();
			mat.data[0] = scale.x;
			mat.data[5] = scale.y;
			mat.data[10] = scale.z;
			return mat;
		}

		/**
		 * Creates a rotation matrix.
		 *
		 * @param xRadians The rotation around the x-axis in radians.
		 * @param yRadians The rotation around the y-axis in radians.
		 * @param zRadians The rotation around the z-axis in radians.
		 *
		 * @returns A rotation matrix.
		 */
		static Matrix4x4 Rotate(f32 xRadians, f32 yRadians, f32 zRadians) {
			Matrix4x4 mat = Matrix4x4::Indentity();

			f32 xSin = Mathf::Sin(xRadians);
			f32 xCos = Mathf::Cos(xRadians);
			f32 ySin = Mathf::Sin(yRadians);
			f32 yCos = Mathf::Cos(yRadians);
			f32 zSin = Mathf::Sin(zRadians);
			f32 zCos = Mathf::Cos(zRadians);

			mat.data[0] = yCos * zCos;
			mat.data[1] = -yCos * zSin;
			mat.data[2] = ySin;
			mat.data[4] = xSin * ySin * zCos + xCos * zSin;
			mat.data[5] = -xSin * ySin * zSin + xCos * zCos;
			mat.data[6] = -xSin * yCos;
			mat.data[8] = -xCos * ySin * zCos + xSin * zSin;
			mat.data[9] = xCos * ySin * zSin + xSin * zCos;
			mat.data[10] = xCos * yCos;

			return mat;
		}

		Matrix4x4 operator*(const Matrix4x4& other) const {
			Matrix4x4 result;
			// Rows
			for (i32 i = 0; i < 4; i++) {
				// Columns
				for (i32 j = 0; j < 4; j++) {
					result.data[i * 4 + j] =
						data[i * 4 + 0] * other.data[0 * 4 + j] + data[i * 4 + 1] * other.data[1 * 4 + j] +
						data[i * 4 + 2] * other.data[2 * 4 + j] + data[i * 4 + 3] * other.data[3 * 4 + j];
				}
			}
			return result;
		}
	};

	struct Quaternion {
		Vector4 vec;

		Quaternion(f32 x, f32 y, f32 z, f32 w) : vec(x, y, z, w) {}

		static Quaternion Identity() { return Quaternion(0.0f, 0.0f, 0.0f, 1.0f); }

		inline void Normalize() { vec.Normalize(); }

		inline Quaternion Normalized() {
			Quaternion quat = Quaternion(vec.x, vec.y, vec.z, vec.w);
			quat.Normalize();
			return quat;
		}

		inline Quaternion Conjugate() { return Quaternion(-vec.x, -vec.y, -vec.z, vec.w); }

		inline Quaternion Inverse() {
			Quaternion quat = Conjugate();
			quat.Normalize();
			return quat;
		}

		static inline f32 Dot(Quaternion& a, Quaternion& b) {
			return a.vec.x * b.vec.x + a.vec.y * b.vec.y + a.vec.z * b.vec.z + a.vec.w * b.vec.w;
		}

		Matrix4x4 ConvertToMatrix() {
			Matrix4x4 mat = Matrix4x4::Indentity();

			mat.data[0] = 1.0f - 2.0f * (vec.y * vec.y + vec.z * vec.z);
			mat.data[1] = 2.0f * (vec.x * vec.y + vec.w * vec.z);
			mat.data[2] = 2.0f * (vec.x * vec.z - vec.w * vec.y);
			mat.data[4] = 2.0f * (vec.x * vec.y - vec.w * vec.z);
			mat.data[5] = 1.0f - 2.0f * (vec.x * vec.x + vec.z * vec.z);
			mat.data[6] = 2.0f * (vec.y * vec.z + vec.w * vec.x);
			mat.data[8] = 2.0f * (vec.x * vec.z + vec.w * vec.y);
			mat.data[9] = 2.0f * (vec.y * vec.z - vec.w * vec.x);
			mat.data[10] = 1.0f - 2.0f * (vec.x * vec.x + vec.y * vec.y);

			return mat;
		}

		Matrix4x4 ConvertToRotationMatrix(Vector3 center) {
			Matrix4x4 out_matrix;

			f32* o = out_matrix.data;
			o[0] = (vec.x * vec.x) - (vec.y * vec.y) - (vec.z * vec.z) + (vec.w * vec.w);
			o[1] = 2.0f * ((vec.x * vec.y) + (vec.z * vec.w));
			o[2] = 2.0f * ((vec.x * vec.z) - (vec.y * vec.w));
			o[3] = center.x - center.x * o[0] - center.y * o[1] - center.z * o[2];

			o[4] = 2.0f * ((vec.x * vec.y) - (vec.z * vec.w));
			o[5] = -(vec.x * vec.x) + (vec.y * vec.y) - (vec.z * vec.z) + (vec.w * vec.w);
			o[6] = 2.0f * ((vec.y * vec.z) + (vec.x * vec.w));
			o[7] = center.y - center.x * o[4] - center.y * o[5] - center.z * o[6];

			o[8] = 2.0f * ((vec.x * vec.z) + (vec.y * vec.w));
			o[9] = 2.0f * ((vec.y * vec.z) - (vec.x * vec.w));
			o[10] = -(vec.x * vec.x) - (vec.y * vec.y) + (vec.z * vec.z) + (vec.w * vec.w);
			o[11] = center.z - center.x * o[8] - center.y * o[9] - center.z * o[10];

			o[12] = 0.0f;
			o[13] = 0.0f;
			o[14] = 0.0f;
			o[15] = 1.0f;

			return out_matrix;
		}

		static Quaternion FromAxisAngle(Vector3 axis, f32 angle) {
			f32 sin = Mathf::Sin(angle / 2.0f);
			f32 cos = Mathf::Cos(angle / 2.0f);

			return Quaternion(axis.x * sin, axis.y * sin, axis.z * sin, cos);
		}

		Quaternion operator*(const Quaternion& other) const {
			return Quaternion(vec.w * other.vec.x + vec.x * other.vec.w + vec.y * other.vec.z - vec.z * other.vec.y,
				vec.w * other.vec.y - vec.x * other.vec.z + vec.y * other.vec.w + vec.z * other.vec.x,
				vec.w * other.vec.z + vec.x * other.vec.y - vec.y * other.vec.x + vec.z * other.vec.w,
				vec.w * other.vec.w - vec.x * other.vec.x - vec.y * other.vec.y - vec.z * other.vec.z);
		}
	};
} // namespace Axle
