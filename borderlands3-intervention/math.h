#pragma once
#include "pch.h"

typedef __m128	VectorRegister;
typedef __m128i VectorRegisterInt;
typedef __m128d VectorRegisterDouble;

#define VectorMultiply(a,b) _mm_mul_ps(a,b)
#define SHUFFLEMASK(A0,A1,B2,B3) ( (A0) | ((A1)<<2) | ((B2)<<4) | ((B3)<<6) )
#define VectorReplicate( Vec, ElementIndex ) _mm_shuffle_ps( Vec, Vec, SHUFFLEMASK(ElementIndex,ElementIndex,ElementIndex,ElementIndex) )
#define VectorMultiplyAdd( Vec1, Vec2, Vec3 ) _mm_add_ps( _mm_mul_ps(Vec1, Vec2), Vec3 )
#define D2R(DEGRESS) DEGRESS*(M_PI/180.0f)

class FVector
{
public:
	float X, Y, Z;

	static const FVector ZeroVector;
	static const FVector OneVector;

	FVector() : X(0.f), Y(0.f), Z(0.f) {}
	FVector(float X, float Y, float Z) : X(X), Y(Y), Z(Z) {}
	FVector(float InF) : X(InF), Y(InF), Z(InF) { }
	float Size() const { return sqrtf(X * X + Y * Y + Z * Z); }
	float DistTo(const FVector& V) const { return (*this - V).Size(); }

	FVector operator+(const FVector& other) const { return FVector(X + other.X, Y + other.Y, Z + other.Z); }
	FVector operator-(const FVector& other) const { return FVector(X - other.X, Y - other.Y, Z - other.Z); }
	FVector operator-(const float& other) const { return FVector(X - other, Y - other, Z - other); }
	FVector operator*(const FVector& V) const { return FVector(X * V.X, Y * V.Y, Z * V.Z); }
	FVector operator/(const FVector& V) const { return FVector(X / V.X, Y / V.Y, Z / V.Z); }
	bool operator!=(const FVector& V) const { return X != V.X && Y != V.Y && Z != V.Z; }
};

class FRotator
{
public:
	float Pitch, Yaw, Roll;

	static const FRotator ZeroRotator;

	FRotator() : Pitch(0.f), Yaw(0.f), Roll(0.f) {}
	FRotator(float Pitch, float Yaw, float Roll) : Pitch(Pitch), Yaw(Yaw), Roll(Roll) {}
	FVector ToVector();
};

class FVector2D
{
public:
	float X, Y;

	static const FVector2D ZeroVector;
	static const FVector2D OneVector;
	static const FVector2D HalfVector;

	FVector2D() : X(0.f), Y(0.f) {}
	FVector2D(float X, float Y) : X(X), Y(Y) {}
	float Size() const { return sqrtf(X * X + Y * Y); }
	float DistTo(const FVector2D& V) const { return (*this - V).Size(); }


	FVector2D operator + (const FVector2D& other) const { return FVector2D(X + other.X, Y + other.Y); }
	FVector2D operator + (float scalar) const { return FVector2D(X + scalar, Y + scalar); }
	FVector2D operator- (const FVector2D& other) const { return FVector2D(X - other.X, Y - other.Y); }
	FVector2D operator- (float scalar) const { return FVector2D(X - scalar, Y - scalar); }
	FVector2D operator* (float scalar) const { return FVector2D(X * scalar, Y * scalar); }
	FVector2D operator/ (float scalar) const { return FVector2D(X / scalar, Y / scalar); }
	friend bool operator>(const FVector2D& one, const FVector2D& two) { return one.X > two.X && one.Y > two.Y; }
	friend bool operator<(const FVector2D& one, const FVector2D& two) { return one.X < two.X && one.Y < two.Y; }
	friend bool operator>=(const FVector2D& one, const FVector2D& two) { return one.X >= two.X && one.Y >= two.Y; }
	friend bool operator<=(const FVector2D& one, const FVector2D& two) { return one.X <= two.X&& one.Y <= two.Y; }
	bool operator!=(const FVector2D& V) const { return X != V.X && Y != V.Y; }
	bool operator==(const FVector2D& V) const { return X == V.X && Y == V.Y; }
};

// ScriptStruct CoreUObject.Quat
// Size: 0x10 (Inherited: 0x00)
class FQuat
{
public:
	float X; // 0x00(0x04)
	float Y; // 0x04(0x04)
	float Z; // 0x08(0x04)
	float W; // 0x0c(0x04)
};

struct FMatrix
{
public:
	union
	{
		__declspec(align(16)) float M[4][4];
	};

	__forceinline void VectorMatrixMultiply(void* Result, const void* Matrix1, const void* Matrix2)
	{
		const VectorRegister* A = (const VectorRegister*)Matrix1;
		const VectorRegister* B = (const VectorRegister*)Matrix2;
		VectorRegister* R = (VectorRegister*)Result;
		VectorRegister Temp, R0, R1, R2, R3;

		// First row of result (Matrix1[0] * Matrix2).
		Temp = VectorMultiply(VectorReplicate(A[0], 0), B[0]);
		Temp = VectorMultiplyAdd(VectorReplicate(A[0], 1), B[1], Temp);
		Temp = VectorMultiplyAdd(VectorReplicate(A[0], 2), B[2], Temp);
		R0 = VectorMultiplyAdd(VectorReplicate(A[0], 3), B[3], Temp);

		// Second row of result (Matrix1[1] * Matrix2).
		Temp = VectorMultiply(VectorReplicate(A[1], 0), B[0]);
		Temp = VectorMultiplyAdd(VectorReplicate(A[1], 1), B[1], Temp);
		Temp = VectorMultiplyAdd(VectorReplicate(A[1], 2), B[2], Temp);
		R1 = VectorMultiplyAdd(VectorReplicate(A[1], 3), B[3], Temp);

		// Third row of result (Matrix1[2] * Matrix2).
		Temp = VectorMultiply(VectorReplicate(A[2], 0), B[0]);
		Temp = VectorMultiplyAdd(VectorReplicate(A[2], 1), B[1], Temp);
		Temp = VectorMultiplyAdd(VectorReplicate(A[2], 2), B[2], Temp);
		R2 = VectorMultiplyAdd(VectorReplicate(A[2], 3), B[3], Temp);

		// Fourth row of result (Matrix1[3] * Matrix2).
		Temp = VectorMultiply(VectorReplicate(A[3], 0), B[0]);
		Temp = VectorMultiplyAdd(VectorReplicate(A[3], 1), B[1], Temp);
		Temp = VectorMultiplyAdd(VectorReplicate(A[3], 2), B[2], Temp);
		R3 = VectorMultiplyAdd(VectorReplicate(A[3], 3), B[3], Temp);

		// Store result
		R[0] = R0;
		R[1] = R1;
		R[2] = R2;
		R[3] = R3;
	};

	__forceinline FMatrix operator*(const FMatrix& Other) {
		FMatrix Result;
		VectorMatrixMultiply(&Result, this, &Other);
		return Result;
	};
};

// ScriptStruct CoreUObject.Transform
// Size: 0x30 (Inherited: 0x00)
class FTransform
{
public:
	FQuat Rotation; // 0x00(0x10)
	FVector Translation; // 0x10(0x0c)
	PAD(0x4); // 0x1c(0x04)
	class FVector Scale3D; // 0x20(0x0c)
	PAD(0x4); // 0x2c(0x04)

	FMatrix ToMatrixWithScale() const;
};

static double RGBToLinear(uint8_t color)
{
	float fColor = (double)(color / 255.0f);

	if (fColor > 0.04045f)
	{
		return pow(fColor * (1.0 / 1.055) + 0.0521327, 2.4);
	}
	else
	{
		return fColor * (1.0 / 12.92);
	}
}

class FLinearColor
{
public:
	float R, G, B, A;

	static const FLinearColor Zero;
	static const FLinearColor outline;

	FLinearColor() : R(0.f), G(0.f), B(0.f), A(0.f) {};
	FLinearColor(float R, float G, float B, float A) : R(R), G(G), B(B), A(A) {};
	FLinearColor(float R, float G, float B) : R(R), G(G), B(B), A(1.0f) {};

	bool operator!=(const FLinearColor& C) const { return R != C.R && G != C.G && B != C.B && A != C.A; }

	static const FLinearColor FromRGB(int R, int G, int B)
	{
		return FLinearColor{ (float)RGBToLinear(R), (float)RGBToLinear(G), (float)RGBToLinear(B), 1.0f};
	}

	static const FLinearColor FromARGB(int A, int R, int G, int B)
	{
		return FLinearColor{ (float)RGBToLinear(R), (float)RGBToLinear(G), (float)RGBToLinear(B), (float)(A / 255.0f) };
	}
};

// ScriptStruct Engine.CanvasUVTri
// 0x0060
class FCanvasUVTri
{
public:
	class FVector2D                                   V0_Pos;                                                   // 0x0000(0x0008) (Edit, BlueprintVisible, IsPlainOldData)
	class FVector2D                                   V0_UV;                                                    // 0x0008(0x0008) (Edit, BlueprintVisible, IsPlainOldData)
	class FLinearColor                                V0_Color;                                                 // 0x0010(0x0010) (Edit, BlueprintVisible, IsPlainOldData)
	class FVector2D                                   V1_Pos;                                                   // 0x0020(0x0008) (Edit, BlueprintVisible, IsPlainOldData)
	class FVector2D                                   V1_UV;                                                    // 0x0028(0x0008) (Edit, BlueprintVisible, IsPlainOldData)
	class FLinearColor                                V1_Color;                                                 // 0x0030(0x0010) (Edit, BlueprintVisible, IsPlainOldData)
	class FVector2D                                   V2_Pos;                                                   // 0x0040(0x0008) (Edit, BlueprintVisible, IsPlainOldData)
	class FVector2D                                   V2_UV;                                                    // 0x0048(0x0008) (Edit, BlueprintVisible, IsPlainOldData)
	class FLinearColor                                V2_Color;                                                 // 0x0050(0x0010) (Edit, BlueprintVisible, IsPlainOldData)
};

class ColorVector2D : public FVector2D
{
public:

	FLinearColor Color;

	ColorVector2D(float x, float y, FLinearColor color)
		: FVector2D(x, y)
	{
		Color = color;
	}

	ColorVector2D(FVector2D location, FLinearColor color)
		: FVector2D(location)
	{
		Color = color;
	}
};

std::pair<float*, float*> minmaxfloat(float* arr, size_t nelem);

void rotate(FVector& point, FRotator& rotation, FVector& out);