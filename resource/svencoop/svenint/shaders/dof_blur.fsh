uniform sampler2D iChannel0; // normal texture
uniform sampler2D depthmap; // depth buffer
uniform int interptype;
uniform float znear;
uniform float zfar;
uniform float distance; // fraction of  minDistance / zFar (which is maxDistance)
uniform float bokeh;
uniform float samples;
uniform float radius;
uniform vec2 dir;
uniform vec2 res;

float curvilinear_value(in float time)
{
	if ( interptype == 0 ) // Simple spline
	{
		float time_squared = time * time;
		return 3.0 * time_squared - 2.0 * time_squared * time;
	}
	else if ( interptype == 1 ) // Parabolic
	{
		return time * time;
	}
	else if ( interptype == 2 ) // Parabolic inverted
	{
		return 2.0 * time - time * time;
	}
	else if ( interptype == 3 ) // Cubic
	{
		float time_squared = time * time;
		return 3.0 * time_squared * time - 2.0 * time_squared * time_squared;
	}
	
	// Linear, no change
	return time;
}

void main()
{
	vec2 uv = vec2(gl_FragCoord.xy / res.xy);
	
	float depth = texture2D(depthmap, uv).x;
	depth = 2.0 * depth - 1.0;
	depth = (2.0 * znear) / (zfar + znear - depth * (zfar - znear));
	
	if ( distance >= depth )
	{
		// gl_FragColor = texture2D(iChannel0, uv);
		return;
	}
	
	float time = (depth - distance) / (1.0 - distance);
	
	// Change linear correlation to curvilinear
	time = curvilinear_value( time );
	
	// Simple spline
	// float time_squared = time * time;
	// time = 3.0 * time_squared - 2.0 * time_squared * time;
	
	vec2 radiusDir = dir * ( radius * ( time ) );
	
	vec4 sum = vec4(0.0);
	vec4 msum = vec4(0.0);

	float delta = 1.0 / samples;
	float di = 1.0 / ( samples - 1.0 );
	
	for (float i = -0.5; i < 0.501; i += di)
	{
		vec4 color = texture2D(iChannel0, uv + (radiusDir * i) / res);
		sum += color * delta;
		msum = max(color, msum);
	}

	gl_FragColor = mix(sum, msum, bokeh);
}