// Set the foillowing according to the way the RayMarching Material is defined in RayMarchingMaterial.cpp
#define USE_AS_TRANSPARENT
//#define USE_AS_OPAQUE
//#define USE_AS_PREPASS

struct Material
{
    sampler3D buffer;
};

uniform Material material;

#ifdef USE_AS_TRANSPARENT
layout (location = 0) out vec4 f_Accumulation;
layout (location = 1) out vec4 f_Revealage;
#endif
#ifdef USE_AS_OPAQUE
layout (location = 0) out vec4 fragColor;
#endif
#ifdef USE_AS_PREPASS
layout (location = 0) out vec4 fragColor;
// These are used for "traditional" depth prepass. Use it for debug here ...
layout (location = 1) out vec4 out_normal;
layout (location = 2) out vec4 out_diffuse;
layout (location = 3) out vec4 out_specular;
#endif

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_texcoord;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec3 in_cameraInModelSpace;

float stepsize = 0.01;

float weight(float z, float alpha) {

     // pow(alpha, colorResistance) : increase colorResistance if foreground transparent are affecting background transparent color
     // clamp(adjust / f(z), min, max) :
     //     adjust : Range adjustment to avoid saturating at the clamp bounds
     //     clamp bounds : to be tuned to avoid over or underflow of the reveleage texture.
     // f(z) = 1e-5 + pow(z/depthRange, orederingStrength)
     //     defRange : Depth range over which significant ordering discrimination is required. Here, 10 camera space units.
     //         Decrease if high-opacity surfaces seem “too transparent”,
     //         increase if distant transparents are blending together too much.
     //     orderingStrength : Ordering strength. Increase if background is showing through foreground too much.
     // 1e-5 + ... : avoid dividing by zero !

     return pow(alpha, 0.5) * clamp(10 / ( 1e-5 + pow(z/10, 6)  ), 1e-2, 3*1e3);
 }

// source: https://github.com/kbinani/colormap-shaders/blob/master/shaders/glsl/MATLAB_jet.frag
float colormap_red(float x) {
    if (x < 0.7) {
        return 4.0 * x - 1.5;
    } else {
        return -4.0 * x + 4.5;
    }
}

float colormap_green(float x) {
    if (x < 0.5) {
        return 4.0 * x - 0.5;
    } else {
        return -4.0 * x + 3.5;
    }
}

float colormap_blue(float x) {
    if (x < 0.3) {
       return 4.0 * x + 0.5;
    } else {
       return -4.0 * x + 2.5;
    }
}

vec4 colormap(float x) {
    float r = clamp(colormap_red(x), 0.0, 1.0);
    float g = clamp(colormap_green(x), 0.0, 1.0);
    float b = clamp(colormap_blue(x), 0.0, 1.0);
    return vec4(r, g, b, 1.0);
}


void main(void) {
	float value;
	vec4 color;
	vec4 frgColor = vec4(0.0);
    bool hit = false;

	// le rayon
	vec3 raypos = in_texcoord;//in_position;
    vec3 camera = in_cameraInModelSpace;
    vec3 raydir =in_position - camera;
    raydir.x /= 4.5;
    raydir = normalize(raydir) * stepsize;

    float accum = 0.;
    // only in OpenGL 4.3 ...
    // int mipmapLevel = textureQueryLevels(material.buffer);

    int mipmapLevel = 6;
//    float currentMipMapLevel = textureQueryLod(material.buffer, in_texcoord).x;

    for(int i = 0;
        i<1000;
        i++) {
        float w = 1.;
        value = textureLod(material.buffer, raypos, 0.).r;
        for (int level = 1; level < mipmapLevel; level++){
            //w = w * 0.5;
            w = w * (mipmapLevel - level)/mipmapLevel;
            value += w * textureLod(material.buffer, raypos, level).r;
        }
        if ( value != 0. )
        {
            accum += value;
            hit = true;
        }

        raypos = raypos + raydir;
        if ( any(greaterThan(raypos, vec3(1.0))) ||
             any(lessThan(raypos, vec3(0.0)))
           )
           break;
    }

    if ( ! hit ) discard;

    // replace the constant 10 by the expectation of the number of photons on the ray ?
    float opacity = clamp(accum/10, 0, 0.9);
#ifdef USE_AS_TRANSPARENT
    float w  = weight(gl_FragCoord.z, opacity);
    f_Accumulation = colormap ( opacity  ) * opacity * w;
    //f_Accumulation = vec4(vec3(opacity), 1);
    f_Revealage = vec4(opacity);
#endif

#ifdef USE_AS_OPAQUE
    float w  = weight(gl_FragCoord.z, opacity);
    fragColor = colormap ( opacity  ) * opacity * w;
    fragColor.a = opacity;

//    fragColor = colormap ( opacity );
//    fragColor.a = opacity;
#endif

#ifdef USE_AS_PREPASS
    fragColor = colormap ( opacity );
    out_normal = vec4(normalize(in_normal), 1.0);
    out_diffuse = vec4(normalize(in_position), 1.0);
    out_specular = vec4(raypos, 1.0);
#endif

}
