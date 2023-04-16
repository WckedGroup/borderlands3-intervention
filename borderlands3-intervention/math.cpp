#include "pch.h"

const FVector FVector::ZeroVector(0.0f, 0.0f, 0.0f);
const FVector FVector::OneVector(1.0f, 1.0f, 1.0f);

const FVector2D FVector2D::ZeroVector(0.0f, 0.0f);
const FVector2D FVector2D::OneVector(1.0f, 1.0f);
const FVector2D FVector2D::HalfVector(0.5f, 0.5f);

const FLinearColor FLinearColor::Zero{ 0.f, 0.f, 0.f, 0.f };
const FLinearColor FLinearColor::outline{ 0.f, 0.f, 0.f, 1.f };

const FRotator FRotator::ZeroRotator(0.0f, 0.0f, 0.0f);

FVector FRotator::ToVector()
{
	float angle, sp, sy, cp, cy;

	angle = D2R(this->Yaw);
	sy = sin(angle);
	cy = cos(angle);

	angle = D2R(this->Pitch*-1);
	sp = sin(angle);
	cp = cos(angle);

	return FVector(cp * cy, cp * sy, -sp);
}

FMatrix FTransform::ToMatrixWithScale() const
{
	FMatrix OutMatrix;
	OutMatrix.M[3][0] = Translation.X;
	OutMatrix.M[3][1] = Translation.Y;
	OutMatrix.M[3][2] = Translation.Z;

	const float x2 = Rotation.X + Rotation.X;
	const float y2 = Rotation.Y + Rotation.Y;
	const float z2 = Rotation.Z + Rotation.Z;
	{
		const float xx2 = Rotation.X * x2;
		const float yy2 = Rotation.Y * y2;
		const float zz2 = Rotation.Z * z2;

		OutMatrix.M[0][0] = (1.0f - (yy2 + zz2)) * Scale3D.X;
		OutMatrix.M[1][1] = (1.0f - (xx2 + zz2)) * Scale3D.Y;
		OutMatrix.M[2][2] = (1.0f - (xx2 + yy2)) * Scale3D.Z;
	}
	{
		const float yz2 = Rotation.Y * z2;
		const float wx2 = Rotation.W * x2;

		OutMatrix.M[2][1] = (yz2 - wx2) * Scale3D.Z;
		OutMatrix.M[1][2] = (yz2 + wx2) * Scale3D.Y;
	}
	{
		const float xy2 = Rotation.X * y2;
		const float wz2 = Rotation.W * z2;

		OutMatrix.M[1][0] = (xy2 - wz2) * Scale3D.Y;
		OutMatrix.M[0][1] = (xy2 + wz2) * Scale3D.X;
	}
	{
		const float xz2 = Rotation.X * z2;
		const float wy2 = Rotation.W * y2;

		OutMatrix.M[2][0] = (xz2 + wy2) * Scale3D.Z;
		OutMatrix.M[0][2] = (xz2 - wy2) * Scale3D.X;
	}

	OutMatrix.M[0][3] = 0.0f;
	OutMatrix.M[1][3] = 0.0f;
	OutMatrix.M[2][3] = 0.0f;
	OutMatrix.M[3][3] = 1.0f;

	return OutMatrix;
}

std::pair<float*, float*> minmaxfloat(float* arr, size_t nelem)
{
	return std::minmax_element(arr, arr + nelem);
}

void rotate(FVector& point, FRotator& rotation, FVector& out)
{
	out.X = 0;
	out.Y = 0;
	out.Z = 0;

	FRotator rRotation{ 0.0f, 0.0f, 0.0f };
	rRotation.Pitch = D2R(rotation.Pitch);
	rRotation.Yaw = D2R(rotation.Yaw);
	rRotation.Roll = D2R(rotation.Roll);

	auto cosa = cos(rRotation.Yaw);
	auto sina = sin(rRotation.Yaw);

	auto cosb = cos(rRotation.Pitch);
	auto sinb = sin(rRotation.Pitch);

	auto cosc = cos(rRotation.Roll);
	auto sinc = sin(rRotation.Roll);

	auto Axx = cosa * cosb;
	auto Axy = cosa * sinb * sinc - sina * cosc;
	auto Axz = cosa * sinb * cosc + sina * sinc;

	auto Ayx = sina * cosb;
	auto Ayy = sina * sinb * sinc + cosa * cosc;
	auto Ayz = sina * sinb * cosc - cosa * sinc;

	auto Azx = -sinb;
	auto Azy = cosb * sinc;
	auto Azz = cosb * cosc;

	out.X = Axx * point.X + Axy * point.Y + Axz * point.Z;
	out.Y = Ayx * point.X + Ayy * point.Y + Ayz * point.Z;
	out.Z = Azx * point.X + Azy * point.Y + Azz * point.Z;
}