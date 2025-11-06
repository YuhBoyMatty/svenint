uniform sampler2D iChannel0;
uniform float size; // blur size is inverted: 1.0 / size
uniform float intensity;
uniform vec2 res;

void main()
{
	vec4 sum = vec4(0.0);
	vec2 uv = vec2(gl_FragCoord.xy / res.xy);

	// Take nine samples, with the distance size between them (horizontal)
	sum += texture2D(iChannel0, uv + ( vec2(-4.0 * size, 0) / res )) * 0.05;
	sum += texture2D(iChannel0, uv + ( vec2(-3.0 * size, 0) / res )) * 0.09;
	sum += texture2D(iChannel0, uv + ( vec2(-2.0 * size, 0) / res )) * 0.12;
	sum += texture2D(iChannel0, uv + ( vec2(-size, 0) / res )) * 0.15;
	sum += texture2D(iChannel0, uv) * 0.16;
	sum += texture2D(iChannel0, uv + ( vec2(size, 0) / res )) * 0.15;
	sum += texture2D(iChannel0, uv + ( vec2(2.0 * size, 0) / res )) * 0.12;
	sum += texture2D(iChannel0, uv + ( vec2(3.0 * size, 0) / res )) * 0.09;
	sum += texture2D(iChannel0, uv + ( vec2(4.0 * size, 0) / res )) * 0.05;

	// Take nine samples, with the distance size between them (vertical)
	sum += texture2D(iChannel0, uv + ( vec2(0, -4.0 * size) / res )) * 0.05;
	sum += texture2D(iChannel0, uv + ( vec2(0, -3.0 * size) / res )) * 0.09;
	sum += texture2D(iChannel0, uv + ( vec2(0, -2.0 * size) / res )) * 0.12;
	sum += texture2D(iChannel0, uv + ( vec2(0, -size) / res )) * 0.15;
	sum += texture2D(iChannel0, uv) * 0.16;
	sum += texture2D(iChannel0, uv + ( vec2(0, size) / res )) * 0.15;
	sum += texture2D(iChannel0, uv + ( vec2(0, 2.0 * size) / res )) * 0.12;
	sum += texture2D(iChannel0, uv + ( vec2(0, 3.0 * size) / res )) * 0.09;
	sum += texture2D(iChannel0, uv + ( vec2(0, 4.0 * size) / res )) * 0.05;

	// Increase blur with intensity
	gl_FragColor = sum * intensity + texture2D(iChannel0, uv);
}