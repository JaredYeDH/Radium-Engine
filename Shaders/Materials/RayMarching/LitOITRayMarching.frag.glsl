layout (location = 0) out vec4 f_Accumulation;
layout (location = 1) out vec4 f_Revealage;

#include "TransformStructs.glsl"
#include "DefaultLight.glsl"
#include "RayMarchingMaterial.glsl"

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_texcoord;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec3 in_tangent;
layout (location = 4) in vec3 in_viewVector;
layout (location = 5) in vec3 in_lightVector;

float stepsize = 1.;


//float weight(float z, float alpha) {

//     // pow(alpha, colorResistance) : increase colorResistance if foreground transparent are affecting background transparent color
//     // clamp(adjust / f(z), min, max) :
//     //     adjust : Range adjustment to avoid saturating at the clamp bounds
//     //     clamp bounds : to be tuned to avoid over or underflow of the reveleage texture.
//     // f(z) = 1e-5 + pow(z/depthRange, orederingStrength)
//     //     defRange : Depth range over which significant ordering discrimination is required. Here, 10 camera space units.
//     //         Decrease if high-opacity surfaces seem “too transparent”,
//     //         increase if distant transparents are blending together too much.
//     //     orderingStrength : Ordering strength. Increase if background is showing through foreground too much.
//     // 1e-5 + ... : avoid dividing by zero !

//     return pow(alpha, 0.5) * clamp(10 / ( 1e-5 + pow(z/10, 6)  ), 1e-2, 3*1e3);
// }

//vec4 shade(vec3 position, in out vec4 value){

//    vec4 color = vec4(vec3(0.),value.a);


//    // Valeur de la normale entre -1 et 1
//    vec3 normal = (value.xyz - 0.5) * 2.0;

//    //vec3 normal_view = normalize(gl_NormalMatrix * normal);

//    // Calcul de la position du fragment dans le repère de la caméra
//    // vec3 position_view = out_mvp_position;//(gl_ModelViewMatrix * vec4 (position, 1.0)).xyz;

//    // direction de la lumière
//    //vec3 lightdir = normalize(gl_LightSource[0].position.xyz - position_view);

//    // vec3 direction = normalize(position - camera);
//    float s = - dot(normal, in_viewVector);

//    if (s > 0.)
//        color = vec4(vec3(s),1.);

//    return color;

//}

vec4 shade(vec3 position, in int value) {
    if ( value > 1)
        return (vec4(0.1, 0., 0., 0.1));
}

void main(void) {
	int value;
	vec4 color;
	vec4 frgColor = vec4(0.0);

	float att=1.0;

	// le rayon
	vec3 position = in_position;
	vec3 direction = - in_viewVector;

    value = texture3D(material.buffer, in_position);

  /*******************************************************************/
  /* Beginning of code to be completed                               */
  /*******************************************************************/


        // boucle pour le ray marching
    for(int i = 0; i< int(4./stepsize); i++) {
        value = texture3D(material.buffer, position);
        if (value >= 1) {

    //test
            // error C7505: OpenGL does not allow swizzles on scalar expressions
            // error C7623: implicit narrowing of type from "vec4" to "int"
            color = shade(position, value); 

    //test
            //frgColor.rgb = color;  // error C7011: implicit cast from "vec4" to "vec3"
            frgColor += color;
            att = 0;
            break;
        }
        position = position + direction * stepsize;
    }

  /*******************************************************************/
  /* Beginning of code to be completed                               */
  /*******************************************************************/

	if (att > 0.99) discard;
	frgColor.a = 1.0;
 	fragColor = frgColor;

    f_Accumulation     = vec4(1.0);
    f_Revealage        = vec4(1.0);
}
