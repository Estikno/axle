#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp";
#include "Mathf.hpp"

namespace Axle {
	struct AXLE_API Vector2 {
		union {
			float x, r, s, u;
		};
		union {
			float y, g, t, v;
		};

		Vector2(float x, float y) : x(x), y(y) {}

		inline Vector2 Zero() {
			return Vector2(0.0f, 0.0f);
		}
		inline Vector2 One() {
			return Vector2(1.0f, 1.0f);
		}
		inline Vector2 Right() {
			return Vector2(1.0f, 0.0f);
		}
		inline Vector2 Left() {
			return Vector2(-1.0f, 0.0f);
		}
		inline Vector2 Up() {
			return Vector2(0.0f, 1.0f);
		}
		inline Vector2 Down() {
			return Vector2(0.0f, -1.0f);
		}

		inline float SqrMagnitude() {
			return x * x + y * y;
		}
		inline float Magnitude() {
			Mathf::Sqrt(SqrMagnitude());
		}

		inline void Normalize() {
			const float magnitude = Magnitude();
			this->x /= magnitude;
			this->y /= magnitude;
		}
		inline Vector2 Normalized() {
			Vector2 vec = Vector2(this->x, this->y);
			vec.Normalize();
			return vec;
		}
		inline Vector2 Perpendicular() {
			return Vector2(-(this->y), this->x);
		}

		static float Distance(Vector2& a, Vector2& b) {
			return (a - b).Magnitude();
		}
		static float Dot(Vector2& a, Vector2& b) {
			return a.x * b.x + a.y * b.y;
		}
		static float Angle(Vector2& a, Vector2& b) {
			return Mathf::Acos(Vector2::Dot(a, b) / (a.Magnitude() * b.Magnitude()));
		}
		static Vector2 LerpUnclamped(Vector2& a, Vector2& b, float t) {
			return a * (1.0f - t) + b * t;
		}
		static Vector2 Lerp(Vector2& a, Vector2& b, float t) {
			return LerpUnclamped(a, b, Mathf::Clamp01(t));
		}
		static Vector2 Reflect(Vector2& inDirection, Vector2& inNormal) {
			return inDirection - inNormal * 2 * Vector2::Dot(inDirection, inNormal);
		}
		static float ScalarProjection(Vector2& toProject, Vector2& onProject) {
			return Vector2::Dot(toProject, onProject) / onProject.Magnitude();
		}
		static Vector2 Projection(Vector2& toProject, Vector2& onProject) {
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
	};

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
	};

	using Quaternion = Vector4;
}
