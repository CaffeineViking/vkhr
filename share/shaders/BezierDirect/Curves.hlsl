//--------------------------------------------------------------------------------------
// Holds all common declarations for compute shaders.
//--------------------------------------------------------------------------------------

#include "Constants.hlsl"
#include "Random.hlsl"

#define UINT_MAX 0xFFFFFFFF 
#define FLOAT_EPSILON 1e-7

#define PI 3.1415926

#define SQRT2 sqrt(2.0)
#define SQRTH sqrt(0.5)

#define FLATNESS_THRESHOLD_AREA 5 // Pixelarea
#define FLATNESS_THRESHOLD_RATIO 5 // Ratio between base line and bounding triangle height.

//--------------------------------------------------------------------------------------
// Unordered Access Buffers
//--------------------------------------------------------------------------------------
// register(u0) is reserved for the render target.
RWTexture2D<float4>						g_BackBuffer				: register(u1);

//--------------------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------------------
struct CurvePrimitive {
	float4 p0;
	float4 p1;
	float4 p2;
};

//--------------------------------------------------------------------------------------
//
//      Swap
//
//		Swap two float values.
//
//--------------------------------------------------------------------------------------
inline void Swap(inout float a, inout float b) {
	float h = a;
	a = b;
	b = h;
}

//--------------------------------------------------------------------------------------
//
//      Swap
//
//		Swap two float4 values.
//
//--------------------------------------------------------------------------------------
inline void Swap(inout float4 a, inout float4 b) {
	float4 h = a;
	a = b;
	b = h;
}

//--------------------------------------------------------------------------------------
//
//      Swap
//
//		Swap two uint values.
//
//--------------------------------------------------------------------------------------
inline void Swap(inout uint a, inout uint b) {
	uint h = a;
	a = b;
	b = h;
}

//--------------------------------------------------------------------------------------
//
//      Swap
//
//		Swap two uint2 values.
//
//--------------------------------------------------------------------------------------
inline void Swap(inout uint2 a, inout uint2 b) {
	uint2 h = a;
	a = b;
	b = h;
}

//--------------------------------------------------------------------------------------
//
//      Swap
//
//		Swap two uint4 values.
//
//--------------------------------------------------------------------------------------
inline void Swap(inout uint4 a, inout uint4 b) {
	uint4 h = a;
	a = b;
	b = h;
}

//--------------------------------------------------------------------------------------
//
//		GetSubCurve
//
//		Creates the subcurve that spans from t0 to t1.
//
//--------------------------------------------------------------------------------------
CurvePrimitive GetSubCurve(CurvePrimitive seg, float t0, float t1) {
	float4 a = seg.p0 - 2 * seg.p1 + seg.p2;
	float4 b = 2 * (seg.p1 - seg.p0);

	CurvePrimitive segOut;
	segOut.p0 = seg.p0 + t0 * (b + a * t0);
	segOut.p2 = seg.p0 + t1 * (b + a * t1);

	float4 qt = seg.p0 + t1 * (seg.p1 - seg.p0);
	segOut.p1 = qt + (t0 / t1) * (segOut.p2 - qt);

	return segOut;
}

//--------------------------------------------------------------------------------------
//
//		GetSubWeights
//
//		Creates the subcurve that spans from t0 to t1.
//
//--------------------------------------------------------------------------------------
float3 GetSubWeights(float3 weights, float t0, float t1) {
	float a = weights.x - 2 * weights.y + weights.z;
	float b = 2 * (weights.y - weights.x);

	float3 weightsOut;
	weightsOut.x = weights.x + t0 * (b + a * t0);
	weightsOut.z = weights.x + t1 * (b + a * t1);

	float qt = weights.x + t1 * (weights.y - weights.x);
	weightsOut.y = qt + (t0 / t1) * (weightsOut.z - qt);
	return weightsOut;
}

//--------------------------------------------------------------------------------------
//
//      GetSubCurve
//
//		Determine the curve that spans from the interval t0 to t1 of the original curve defined by a, b and p0.
//		p0 must be the point at p(t0) on the curve.
//
//--------------------------------------------------------------------------------------
inline CurvePrimitive GetSubCurve(float4 p0, float t0, float h, float4 a, float4 b)
{
	// Derivative of the original curve p'(t0)
	float4 dp = 2 * a * t0 + b;
	// Derivative of the original curve at p'(t0 + h)
	float4 dph = dp + 2 * a * h;

	CurvePrimitive curveOut;
	curveOut.p0 = p0;
	// Construct p1 by going along the tangent at p(t0) starting from p0
	curveOut.p1 = p0 + 0.5 * h * dp;
	// Construct p2 by going along the tangent at p(t0 + h) starting from p1
	curveOut.p2 = curveOut.p1 + 0.5 * h * dph;
	return curveOut;
}

//--------------------------------------------------------------------------------------
//
//      PreservativePerspectiveDivision
//
//		Performs the perspective division. 
//		Saves the reciprocal of the old w at the w component of the position vector.
//
//--------------------------------------------------------------------------------------
inline float4 PreservativePerspectiveDivision(float4 pos) {
	float rcpw = rcp(pos.w);
	pos *= rcpw;
	pos.w = rcpw;
	return pos;
}

//--------------------------------------------------------------------------------------
//
//      HomogeneousClipSpacePosToScreenPos
//
//		Transform a vector in homogeneous clip space to the screen space.
//		--> First does perspective division to get the normalized device coordinates.
//		--> Then scales the coordinates to the whole screen.
//
//--------------------------------------------------------------------------------------
inline float4 HomogeneousClipSpacePosToScreenPos(float4 pos)
{
	pos = mul(pos, g_mClipToViewport);
	return PreservativePerspectiveDivision(pos);
}

//--------------------------------------------------------------------------------------
//
//      ToScreenSpace
//
//		Transform a bezier curve given in clip space to screen space.
//
//--------------------------------------------------------------------------------------
inline void ToScreenSpace(inout CurvePrimitive curve) {
	curve.p0 = HomogeneousClipSpacePosToScreenPos(curve.p0);
	curve.p1 = HomogeneousClipSpacePosToScreenPos(curve.p1);
	curve.p2 = HomogeneousClipSpacePosToScreenPos(curve.p2);
}


//--------------------------------------------------------------------------------------
//
//      WorldPosToHomogeneousClipSpacePos
//
//		Transform a vector in world space to homogeneous clip space.
//		--> Does the (Model)ViewProjection multiplication
//		--> No perspective division yet
//
//--------------------------------------------------------------------------------------
inline float4 WorldPosToHomogeneousClipSpacePos(float4 worldPos)
{
	worldPos = mul(float4(worldPos.xyz, 1), g_mViewProj);
	return mul(worldPos, g_mClipSpace);
}

//--------------------------------------------------------------------------------------
//
//      ScreenPosToWorldPos
//
//		Transform a vector from screen space back to world space.
//
//--------------------------------------------------------------------------------------
inline float3 ScreenPosToWorldPos(float3 screenPos)
{
	float4 worldPos = mul(float4(screenPos, 1), g_mInvViewProjViewport);
	return worldPos.xyz / worldPos.w;
}

//--------------------------------------------------------------------------------------
//
//      ScreenToWorld
//
//		Transform a reference point as well as a direction vector that originates at the reference point (e.g. a normal or a tangent) 
//		from screen space back to world space. The output direction is not normalized.
//
//--------------------------------------------------------------------------------------
inline void ScreenToWorld(inout float3 referencePoint, inout float3 direction)
{
	float3 endPoint = referencePoint + direction;

	referencePoint.z = rcp(referencePoint.z);
	endPoint.z = rcp(endPoint.z);
	
	referencePoint = ScreenPosToWorldPos(referencePoint);
	endPoint = ScreenPosToWorldPos(endPoint);
	direction = endPoint - referencePoint;
}

//--------------------------------------------------------------------------------------
//
//      ToClipSpace
//
//		Transform a bezier curve given in world space to clip space.
//
//--------------------------------------------------------------------------------------
inline void ToClipSpace(inout CurvePrimitive seg) {
	seg.p0 = WorldPosToHomogeneousClipSpacePos(seg.p0);// Transform coordinates to clip space
	seg.p1 = WorldPosToHomogeneousClipSpacePos(seg.p1);// Transform coordinates to clip space
	seg.p2 = WorldPosToHomogeneousClipSpacePos(seg.p2);// Transform coordinates to clip space
}

//--------------------------------------------------------------------------------------
//
//      ViewProj
//
//		Multiply each control point position with the view projection transformation matrix.
//
//--------------------------------------------------------------------------------------
inline void ViewProj(inout CurvePrimitive curve) {
	curve.p0 = mul(float4(curve.p0.xyz, 1), g_mViewProj);
	curve.p1 = mul(float4(curve.p1.xyz, 1), g_mViewProj);
	curve.p2 = mul(float4(curve.p2.xyz, 1), g_mViewProj);
}

//--------------------------------------------------------------------------------------
//
//      ProjectRadius
//
//		Project the radius at a certain depth (non linear).
//		make sure to use the correct depth: 
//		https://msdn.microsoft.com/en-us/library/windows/desktop/bb206341(v=vs.85).aspx
//		--> Depth must be the z component of the position in view space.
//
//--------------------------------------------------------------------------------------
inline float ProjectRadius(float depth, float tanHalfFov, float halfScreenHeight) {
	// Project radii
	// http://stackoverflow.com/questions/21648630/radius-of-projected-sphere-in-screen-space
	// Summary: projectedR =  r / ( sqrt(d ^ 2 - r ^ 2) * tan(fovY / 2))

	float radius = g_HalfHairThickness;
	radius /= sqrt(depth * depth - radius * radius) * tanHalfFov;
	return radius * halfScreenHeight;
}

//--------------------------------------------------------------------------------------
//
//      ProjectHairRadius
//
//--------------------------------------------------------------------------------------
inline void ProjectHairRadius(inout CurvePrimitive curve, float3 radiusWeights) {

	CurvePrimitive depthProvider = curve;
	depthProvider.p0.xyz = ScreenPosToWorldPos(curve.p0.xyz);
	depthProvider.p1.xyz = ScreenPosToWorldPos(curve.p1.xyz);
	depthProvider.p2.xyz = ScreenPosToWorldPos(curve.p2.xyz);
	ViewProj(depthProvider);

	float tanHalfFov = tan(g_FieldOfView * 0.5);
	float halfScreenHeight = g_WinSize.y * 0.5;

	curve.p0.w = abs(ProjectRadius(depthProvider.p0.z, tanHalfFov, radiusWeights.x * halfScreenHeight));
	curve.p1.w = abs(ProjectRadius(depthProvider.p1.z, tanHalfFov, radiusWeights.y * halfScreenHeight));
	curve.p2.w = abs(ProjectRadius(depthProvider.p2.z, tanHalfFov, radiusWeights.z * halfScreenHeight));
}

//--------------------------------------------------------------------------------------
//
//      PrepareInterpolation
//
//		Prepare the curves z and w component holding depth and radius information for
//		perspective correct interpolation.
//
//--------------------------------------------------------------------------------------
inline void PrepareInterpolation(inout CurvePrimitive curve) {
	curve.p0.z = rcp(curve.p0.z);
	curve.p1.z = rcp(curve.p1.z);
	curve.p2.z = rcp(curve.p2.z);

	curve.p0.w *= curve.p0.z;
	curve.p1.w *= curve.p1.z;
	curve.p2.w *= curve.p2.z;
}

//--------------------------------------------------------------------------------------
//
//      ToScreenSpace
//
//		Transform a bezier curve given in clip space to screen space.
//		Performs the clipping using t0 and t1 abd prepares the curve 
//		for perspective correct interpolation.
//		The weights are defined per world space control point, 
//		so they have to be interpolated as well.
//
//--------------------------------------------------------------------------------------
inline void ToScreenSpace(inout CurvePrimitive curve, float t0, float t1, float3 radiusWeights) {
	curve = GetSubCurve(curve, t0, t1);
	radiusWeights = GetSubWeights(radiusWeights, t0, t1);
	ToScreenSpace(curve);
	ProjectHairRadius(curve, radiusWeights);
	PrepareInterpolation(curve);
}

//--------------------------------------------------------------------------------------
//
//      ComputeShading
//
//		Computes shading based on the Kajiya-Kay Model [Kajiya 84] 
//		and Marschner dual highlight [Marschner 03].
//		Taken from TressFX with minor adjustments to fit the coding style.
//
//--------------------------------------------------------------------------------------
float3 ComputeShading(float3 fragmentPosition, float3 fragmentTangent)
{
	// Create a random value for the flickering effect of the secondary highlight.
	float randomValue = Random(uint2(fragmentPosition.xy)).x / (float)UINT_MAX;

	// Transform fragmentPosition and fragmentTangent to world space.
	ScreenToWorld(fragmentPosition, fragmentTangent);

	// Define hair material coefficients and exponents.
	float Ka = g_MatKValue.x, Kd = g_MatKValue.y,
		Ks1 = g_MatKValue.z, Ex1 = g_SpecularExponents.x,
		Ks2 = g_MatKValue.w, Ex2 = g_SpecularExponents.y;

	float3 lightPos = g_PointLightPos.xyz;
	float3 lightDir = normalize(lightPos - fragmentPosition.xyz);
	float3 eyeDir = normalize(g_EyePos.xyz - fragmentPosition.xyz);
	float3 tangent = normalize(fragmentTangent.xyz);

	// In Kajiya's model: diffuse component: sin(t, l)
	float cosTL = (dot(tangent, lightDir));
	float sinTL = sqrt(1 - cosTL*cosTL);
	// Here sinTL is apparently larger than 0
	float diffuse = sinTL;

	// Tilted angle (0 - 10 degree)
	float alpha = (randomValue * 10) * PI / 180;

	// In Kajiya's model: specular component: cos(t, rl) * cos(t, e) + sin(t, rl)sin(t, e)
	float cosTRL = -cosTL;
	float sinTRL = sinTL;
	float cosTE = (dot(tangent, eyeDir));
	float sinTE = sqrt(1 - cosTE*cosTE);

	// Primary highlight: reflected direction shift towards root (2 * Alpha)
	float cosTRL_root = cosTRL * cos(2 * alpha) - sinTRL * sin(2 * alpha);
	float sinTRL_root = sqrt(1 - cosTRL_root * cosTRL_root);
	float specular_root = max(0, cosTRL_root * cosTE + sinTRL_root * sinTE);

	// Secondary highlight: reflected direction shifted toward tip (3*Alpha)
	float cosTRL_tip = cosTRL*cos(-3 * alpha) - sinTRL*sin(-3 * alpha);
	float sinTRL_tip = sqrt(1 - cosTRL_tip * cosTRL_tip);
	float specular_tip = max(0, cosTRL_tip * cosTE + sinTRL_tip * sinTE);

	// Completely in shadow looks very flat, so don't go below some min value
	float amountLight = 1.0; // No shadow lookup for now -> Full visibility. 
							 // TODO (Maybe approximate by sphere around head center - the amount of sphere crossed to get to the light the darker the hair?)

	float3 outColor = Ka * g_AmbientLightColor.xyz * g_HairBaseColor.xyz + // ambient
		amountLight * g_PointLightColor.xyz * (
			Kd * diffuse * g_HairBaseColor.xyz + // diffuse
			Ks1 * pow(specular_root, Ex1) + // primary hightlight r
			Ks2 * pow(specular_tip, Ex2) * g_HairBaseColor.xyz); // secondary highlight rtr

	return saturate(outColor);
}


//--------------------------------------------------------------------------------------
//
//		IsFlat
//
//		Checks if a curve is almost flat.
//
//--------------------------------------------------------------------------------------
inline bool IsFlat(CurvePrimitive curve) {
	// Check for triangle area, this naturally serves as LOD as well.
	// Area Triangle = 0.5 * length(cross(v1, v2))
	float2 a = curve.p1.xy - curve.p0.xy;
	float2 v = curve.p2.xy - curve.p0.xy;

	float c = abs(a.x * v.y - a.y * v.x);
	if (c < 2 * FLATNESS_THRESHOLD_AREA) {
		return true;
	}

	// We do an additional flatness test as well (ratio of triangle base to triangle height) 
	// to capture long almost straight curves that can overlap many pixels, too. 

	float baseLength = dot(v, v);

	// Project p1 onto p0p2
	float2 pp1 = v * dot(a, v) / baseLength;
	pp1 -= curve.p1.xy;
	float height = dot(pp1, pp1);

	return baseLength > FLATNESS_THRESHOLD_RATIO * height;
}

//--------------------------------------------------------------------------------------
//
//		ComputeTangent
//
//		Computes the tangent of a given quadratic bezier curve.
//
//--------------------------------------------------------------------------------------
inline float3 ComputeTangent(CurvePrimitive curvePrimitive, float t)
{
	float c0 = t - 1;
	float c1 = -2 * t + 1;
	return  c0 * curvePrimitive.p0.xyz + c1 * curvePrimitive.p1.xyz + t * curvePrimitive.p2.xyz;
}
