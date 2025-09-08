uniform sampler2D iChannel0;
uniform float bokeh;
uniform float samples;
uniform vec2 dir;
uniform vec2 res;

void main()
{
	vec2 uv = vec2(gl_FragCoord.xy / res.xy);
	
	vec4 sum = vec4(0.0); // resulting color
	vec4 msum = vec4(0.0); // the highest value of color portions

	float delta = 1.0 / samples; // portion of color in one sample
	float di = 1.0 / ( samples - 1.0 ); // calc increment
	
	for (float i = -0.5; i < 0.501; i += di)
	{
		vec4 color = texture2D(iChannel0, uv + (dir * i) / res); // make a portion in a given direction
		sum += color * delta; // sum colors
		msum = max(color, msum); // calc the highest value of color
	}

	gl_FragColor = mix(sum, msum, bokeh); // mix the resulting color with the maximum one in a given proportion
}