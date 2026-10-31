#version 330 core
in vec3 position;
in vec3 normal;
in vec4 color;
in vec2 uv;
uniform vec3 eye;
uniform vec3 torch_direction;
uniform vec3 lights[4];
uniform int enabled[4];
uniform bool textured;
uniform bool environment;
uniform float emission;
uniform sampler2D image;
out vec4 fragColor;
void main() {
 if(environment) {
   vec3 d=normalize(position);
   vec2 st=vec2(atan(d.z,d.x)/6.2831853+0.5,asin(d.y)/3.1415927+0.5);
   fragColor=texture(image,st); return;
 }
 vec3 base=color.rgb*(textured?texture(image,uv).rgb:vec3(1));
 vec3 n=normalize(normal), light=vec3(0.18,0.19,0.2), spec=vec3(0);
 for(int i=0;i<4;i++) if(enabled[i]!=0) {
   vec3 delta=lights[i]-position; float distance=length(delta);
   vec3 l=normalize(delta);
   vec3 tint=i==0?vec3(1,0.83,0.65):i==1?vec3(0.75,0.85,1):(i==2?vec3(1,0.65,0.28):vec3(0.7,0.85,1));
   float attenuation=(i<2?2.2:i==2?1.2:4.0)/(1+0.08*distance+0.025*distance*distance);
   if(i==3) attenuation*=smoothstep(cos(radians(30.0)),cos(radians(16.0)),dot(-l,torch_direction));
   light+=tint*max(dot(n,l),0)*attenuation;
   if(dot(n,l)>0) spec+=tint*pow(max(dot(n,normalize(l+normalize(eye-position))),0),32)*0.15*attenuation;
 }
 vec3 lit=base*(light+vec3(emission))+spec;
 fragColor=vec4(pow(vec3(1)-exp(-lit*1.1),vec3(0.78)),1);
}
