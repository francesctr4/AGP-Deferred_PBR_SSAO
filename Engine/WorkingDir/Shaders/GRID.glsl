///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef GRID

#if defined(VERTEX) ///////////////////////////////////////////////////

layout (location = 0) in vec3 position;

out vec2 texCoord;

void main()
{
	texCoord = position.xy * 0.5 + vec2(0.5);
	gl_Position = vec4(position, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

uniform float left;
uniform float right;
uniform float bottom;
uniform float top;
uniform float znear;
uniform mat4 worldMatrix;
uniform mat4 viewMatrix;

in vec2 texCoord;
out vec4 outColor;

// Grid Lines
float grid(vec3 worldPos, float gridStep)
{
	// Compute world-space grid lines
	vec2 grid = fwidth(worldPos.xz) / mod(worldPos.xz, gridStep);
	float line = step(1.0, max(grid.x, grid.y));
	return line;
}

void main()
{
	outColor = vec4(1.0);

	// Eye direction
	vec3 eyedirEyespace;
	eyedirEyespace.x = left + texCoord.x * (right - left);
	eyedirEyespace.y = bottom + texCoord.y * (top - bottom);
	eyedirEyespace.z = -znear;
	vec3 eyedirWorldspace = normalize(mat3(worldMatrix) * eyedirEyespace);

	// Eye position
	vec3 eyeposEyespace = vec3(0.0);
	vec3 eyeposWorldspace = vec3(worldMatrix * vec4(eyeposEyespace, 1.0));

	// Plane parameters
	vec3 planeNormalWorldspace = vec3(0.0, 1.0, 0.0);
	vec3 planePointWorldspace = vec3(0.0, 0.0, 0.0);

	// Ray-plane intersection
	float numerator = dot(planePointWorldspace - eyeposWorldspace, planeNormalWorldspace);
	float denominator = dot(eyedirWorldspace, planeNormalWorldspace);
	float t = numerator / denominator;

	if (t > 0.0) // Intersected in front of the eye
	{
		vec3 hitWorldspace = eyeposWorldspace + eyedirWorldspace * t;
		outColor = vec4(grid(hitWorldspace, 1.0));
	}
	else 
	{
		gl_FragDepth = 0.0;
		discard;
	}
}

#endif
#endif