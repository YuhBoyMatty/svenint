glow_enabled = true;
glow_rgb_update = 0.05;
glow_rgb_hue = 0.0;
glow_rgb_hue_delta = 0.015;
glow_rgb_saturation = 0.8;
glow_rgb_lightness = 0.5;

function GlowRGB()
    if ( not glow_enabled ) then
        return;
    end

    local r, g, b;

	r, g, b = UTIL_HSL2RGB( glow_rgb_hue, glow_rgb_saturation, glow_rgb_lightness );

	glow_rgb_hue = glow_rgb_hue + glow_rgb_hue_delta;

	while ( glow_rgb_hue > 1.0 ) do
		glow_rgb_hue = glow_rgb_hue - 1.0;
    end

    glow_rgb_saturation = glow_rgb_saturation + 0.005;

	while ( glow_rgb_saturation > 1.0 ) do
		glow_rgb_saturation = glow_rgb_saturation - 1.0;
    end

    ClientCmd( string.format( ".glow %d %d %d", math.floor( 255.0 * r ), math.floor( 255.0 * g ), math.floor( 255.0 * b ) ) );

    CreateTimer( glow_rgb_update, GlowRGB );
end

function UTIL_HSL2RGB( h, s, l )
    local r, g, b;

	if ( s == 0.0 ) then
        r = l;
        g = l;
        b = l;

		return r, g, b;
    end

	local q = l < 0.5 and l * (1.0 + s) or l + s - l * s;
	local p = 2.0 * l - q;

	r = UTIL_Hue2RGB( p, q, h + (1.0 / 3.0) );
	g = UTIL_Hue2RGB( p, q, h );
	b = UTIL_Hue2RGB( p, q, h - (1.0 / 3.0) );

    return r, g, b;
end

function UTIL_Hue2RGB( p, q, t )
	if ( t < 0.0 ) then
		t = t + 1.0;
    end

	if ( t > 1.0 ) then
		t = t - 1.0;
    end

	if ( t < 1.0 / 6.0 ) then
		return p + (q - p) * 6.0 * t;
    end

	if ( t < 1.0 / 2.0 ) then
		return q;
    end

	if ( t < 2.0 / 3.0 ) then
		return p + (q - p) * ( (2.0 / 3.0) - t ) * 6.0;
    end

	return p;
end

GlowRGB();