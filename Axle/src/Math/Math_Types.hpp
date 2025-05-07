#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp";
#include "Mathf.hpp"

namespace Axle {
	/// Representation of 2D vectors and points
	struct AXLE_API Vector2 {
		union {
			float x, r, s, u;
		};
		union {
			float y, g, t, v;
		};

		Vector2(float x, float y) : x(x), y(y) {}

		/// Shorthand of writing Vector2(0.0f, 0.0f)
		inline Vector2 Zero() {
			return Vector2(0.0f, 0.0f);
		}
		/// Shorthand of writing Vector2(1.0f, 1.0f)
		inline Vector2 One() {
			return Vector2(1.0f, 1.0f);
		}
		/// Shorthand of writing Vector2(1.0f, 0.0f)
		inline Vector2 Right() {
			return Vector2(1.0f, 0.0f);
		}
		/// Shorthand of writing Vector2(-1.0f, 0.0f)
		inline Vector2 Left() {
			return Vector2(-1.0f, 0.0f);
		}
		/// Shorthand of writing Vector2(0.0f, 1.0f)
		inline Vector2 Up() {
			return Vector2(0.0f, 1.0f);
		}
		/// Shorthand of writing Vector2(0.0f, -1.0f)
		inline Vector2 Down() {
			return Vector2(0.0f, -1.0f);
		}

		/**
		* Returns the squared magnitude of the vector
		* 
		* It's helpful if you don't need to compare the magnitudes directly because doing the square root is computer intensive.
		* 
		* @returns The squared magnitude of the vector
		*/
		inline float SqrMagnitude() {
			return x * x + y * y;
		}

		/**
		* Returns the magnitude of the vector
		* 
		* If you only need to compare magnitudes of some vectors, you can use SsqrMagnitude (computing squared magnitudes is faster)
		* 
		* @returns The magnitude of the vector
		*/
		inline float Magnitude() {
			Mathf::Sqrt(SqrMagnitude());
		}

		/// Modifies the current vector to have a magnitude of 1
		inline void Normalize() {
			const float magnitude = Magnitude();
			this->x /= magnitude;
			this->y /= magnitude;
		}

		/**
		* Returns a new vector with the same direction as the current one, but with a magnitude of 1
		* 
		* @returns A new vector with the same direction but with a magnitude of 1
		*/
		inline Vector2 Normalized() {
			Vector2 vec = Vector2(this->x, this->y);
			vec.Normalize();
			return vec;
		}

		/**
		* Returns a new vector that is perpendicular to the current vector
		* 
		* The result is always rotated 90-degrees in a counter-clockwise direction for a 2D coordinate system where the positive Y axis goes up.
		*
		* @returns A new vector that is perpendicular to the current vector. 
		*/
		inline Vector2 Perpendicular() {
			return Vector2(-(this->y), this->x);
		}

		/**
		* Calculates the euclidean distance between two vectors
		* 
		* @param a The first vector
		* @param b The second vector
		* 
		* @returns The distance between two vectors
		*/
		static float Distance(Vector2& a, Vector2& b) {
			return (a - b).Magnitude();
		}

		/**
		* Calculates the dot product of two vectors
		* 
		* @param a The first vector
		* @param b The second vector
		*
		* @returns The dot product of the two vectors
		*/
		static float Dot(Vector2& a, Vector2& b) {
			return a.x * b.x + a.y * b.y;
		}

		/**
		* Calculates the angle in radians between two vectors
		* 
		* @param a The first vector
		* @param b The second vector
		*
		* @returns The angle in radians between two vectors
		*/
		static float Angle(Vector2& a, Vector2& b) {
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
		static Vector2 LerpUnclamped(Vector2& a, Vector2& b, float t) {
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
		static Vector2 Lerp(Vector2& a, Vector2& b, float t) {
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
		static Vector2 Reflect(Vector2& inDirection, Vector2& inNormal) {
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
		static float ScalarProjection(Vector2& toProject, Vector2& onProject) {
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
		static Vector2 Project(Vector2& toProject, Vector2& onProject) {
			return onProject.Normalized() * ScalarProjection(toProject, onProject);
		}

		Vector2 operator+(const Vector2& other) const {
			return Vector2(x + other.x, y + other.y);
		}
		Vector2 operator-(const Vector2& other) const {
			return Vector2(x - other.x, y - other.y);
		}
		Vector2 operator*(const float& other) const {
			return Vector2(x * other, y * other);
		}
		Vector2 operator/(const float& other) const {
			return Vector2(x / other, y / other);
		}
		bool operator==(const Vector2& other) const {
			return Mathf::Approximately(x, other.x) && Mathf::Approximately(y, other.y);
		}
	};

	/// Representation of 3D vectors and points
	struct AXLE_API Vector3 {
		union {
			float x, r, s, u;
		};
		union {
			float y, g, t, v;
		};
		union {
			float z, b, p, w;
		};

		Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

		/**
		* Converts the current vector3 to a vector4
		* 
		* @param w The component w of the new Vector4
		* 
		* @returns The converted Vector4
		*/
		Vector4 ConvertToVector4(float w) {
			return Vector4(this->x, this->y, this->z, w);
		}

		/// Shorthand of writing Vector3(0.0f, 0.0f, 0.0f)
		inline Vector3 Zero() {
			return Vector3(0.0f, 0.0f, 0.0f);
		}
		/// Shorthand of writing Vector3(1.0f, 1.0f, 1.0f)
		inline Vector3 One() {
			return Vector3(1.0f, 1.0f, 1.0f);
		}
		/// Shorthand of writing Vector3(1.0f, 0.0f, 0.0f)
		inline Vector3 Right() {
			return Vector3(1.0f, 0.0f, 0.0f);
		}
		/// Shorthand of writing Vector3(-1.0f, 0.0f, 0.0f)
		inline Vector3 Left() {
			return Vector3(-1.0f, 0.0f, 0.0f);
		}
		/// Shorthand of writing Vector3(0.0f, 1.0f, 0.0f)
		inline Vector3 Up() {
			return Vector3(0.0f, 1.0f, 0.0f);
		}
		/// Shorthand of writing Vector3(0.0f, -1.0f, 0.0f)
		inline Vector3 Down() {
			return Vector3(0.0f, -1.0f, 0.0f);
		}
		/// Shorthand of writing Vector3(0.0f, 0.0f, 1.0f)
		inline Vector3 Forward() {
			return Vector3(0.0f, 0.0f, 1.0f);
		}
		/// Shorthand of writing Vector3(0.0f, 0.0f, -1.0f)
		inline Vector3 Back() {
			return Vector3(0.0f, 0.0f, -1.0f);
		}

		/**
		* Returns the squared magnitude of the vector
		* 
		* It's helpful if you don't need to compare the magnitudes directly because doing the square root is computer intensive.
		* 
		* @returns The squared magnitude of the vector
		*/
		inline float SqrMagnitude() {
			return x * x + y * y + z * z;
		}

		/**
		* Returns the magnitude of the vector
		* 
		* If you only need to compare magnitudes of some vectors, you can use SsqrMagnitude (computing squared magnitudes is faster)
		* 
		* @returns The magnitude of the vector
		*/
		inline float Magnitude() {
			Mathf::Sqrt(SqrMagnitude());
		}

		/// Modifies the current vector to have a magnitude of 1
		inline void Normalize() {
			const float magnitude = Magnitude();
			this->x /= magnitude;
			this->y /= magnitude;
			this->z /= magnitude;
		}

		/**
		* Returns a new vector with the same direction as the current one, but with a magnitude of 1
		* 
		* @returns A new vector with the same direction but with a magnitude of 1
		*/
		inline Vector3 Normalized() {
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
		static float Distance(Vector3& a, Vector3& b) {
			return (a - b).Magnitude();
		}

		/**
		* Calculates the dot product of two vectors
		* 
		* @param a The first vector
		* @param b The second vector
		*
		* @returns The dot product of the two vectors
		*/
		static float Dot(Vector3& a, Vector3& b) {
			return a.x * b.x + a.y * b.y + a.z * b.z;
		}

		/**
		* Calculates the angle in radians between two vectors
		* 
		* @param a The first vector
		* @param b The second vector
		*
		* @returns The angle in radians between two vectors
		*/
		static float Angle(Vector3& a, Vector3& b) {
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
		static Vector3 LerpUnclamped(Vector3& a, Vector3& b, float t) {
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
		static Vector3 Lerp(Vector3& a, Vector3& b, float t) {
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
		static Vector3 Reflect(Vector3& inDirection, Vector3& inNormal) {
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
		static float ScalarProjection(Vector3& toProject, Vector3& onProject) {
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
		static Vector3 Project(Vector3& toProject, Vector3& onProject) {
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
		static Vector3 Cross(Vector3& a, Vector3& b) {
			return Vector3(a.y * b.z - b.y * a.z, b.x * a.z - a.x * b.z, a.x * b.y - b.x * a.y);
		}

		/**
		* Projects a vector onto a plane.
		* 
		* For a given plane described by planeNormal and a given vector vector, the method generates a new vector orthogonal to planeNormal and parallel to the plane. 
		* Note: planeNormal does not need to be normalized
		* 
		* @param vector The vector to be projected
		* @param planeNormal The normal which defines the plane
		* 
		* @returns The orthogonal projection of vector on the plane
		*/
		static Vector3 ProjectOnPlane(Vector3& vector, Vector3& planeNormal) {
			Vector3 projectionOnNormal = Project(vector, planeNormal);
			return vector - projectionOnNormal;
		}

		Vector3 operator+(const Vector3& other) const {
			return Vector3(x + other.x, y + other.y, z + other.z);
		}
		Vector3 operator-(const Vector3& other) const {
			return Vector3(x - other.x, y - other.y, z - other.z);
		}
		Vector3 operator*(const float& other) const {
			return Vector3(x * other, y * other, z * other);
		}
		Vector3 operator/(const float& other) const {
			return Vector3(x / other, y / other, z / other);
		}
		bool operator==(const Vector3& other) const {
			return Mathf::Approximately(x, other.x) && Mathf::Approximately(y, other.y) && Mathf::Approximately(z, other.z);
		}
	};

	/// Representation of 4D vectors and points
	struct AXLE_API Vector4 {
		union {
			float x, r, s;
		};
		union {
			float y, g, t;
		};
		union {
			float z, b, p;
		};
		union {
			float w, a, q;
		};

		Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

		/// Shorthand of writing Vector4(0.0f, 0.0f, 0.0f, 0.0f)
		Vector4 Zero() {
			return Vector4(0.0f, 0.0f, 0.0f, 0.0f);
		}

		/// Shorthand of writing Vector4(1.0f, 1.0f, 1.0f, 1.0f)
		Vector4 One() {
			return Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		/**
		* Returns the squared magnitude of the vector
		* 
		* It's helpful if you don't need to compare the magnitudes directly because doing the square root is computer intensive.
		* 
		* @returns The squared magnitude of the vector
		*/
		inline float SqrMagnitude() {
			return x * x + y * y + z * z + w * w;
		}

		/**
		* Returns the magnitude of the vector
		* 
		* If you only need to compare magnitudes of some vectors, you can use SsqrMagnitude (computing squared magnitudes is faster)
		* 
		* @returns The magnitude of the vector
		*/
		inline float Magnitude() {
			Mathf::Sqrt(SqrMagnitude());
		}

		/// Modifies the current vector to have a magnitude of 1
		inline void Normalize() {
			const float magnitude = Magnitude();
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
		Vector3 ConvertToVector3() {
			return Vector3(this->x, this->y, this->z);
		}

		/**
		* Converts the current vector4 to a vector2
		* 
		* @returns The converted Vector2
		*/
		Vector2 ConvertToVector2() {
			return Vector2(this->x, this->y);
		}
		
		/**
		* Calculates the dot product of two vectors
		* 
		* @param a The first vector
		* @param b The second vector
		*
		* @returns The dot product of the two vectors
		*/
		static float Dot(Vector4& a, Vector4& b) {
			return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
		}

		Vector4 operator+(const Vector4& other) const {
			return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
		}
		Vector4 operator-(const Vector4& other) const {
			return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
		}
		Vector4 operator*(const float& other) const {
			return Vector4(x * other, y * other, z * other, w * other);
		}
		Vector4 operator/(const float& other) const {
			return Vector4(x / other, y / other, z / other, w / other);
		}
		bool operator==(const Vector4& other) const {
			return Mathf::Approximately(x, other.x) && Mathf::Approximately(y, other.y) && Mathf::Approximately(z, other.z) && Mathf::Approximately(w, other.w);
		}

	};

	using Quaternion = Vector4;
}
