//
//  shaderOFx.h
//  curiositas2017
//
//  Created by FatDazz_mac on 14/04/2017.
//
//
#pragma once

#define STRINGIFY(A) #A

#include "ofMain.h"
#include "ofxFXObject.h"

class ofxBoidFx : public ofxFXObject {
public:
    ofxBoidFx(){
        passes = 1;
        internalFormat = GL_RGBA;
        fragmentShader = STRINGIFY(
                                   uniform vec2 resolution;
                                   uniform vec2 tab[1000];
                                   //uniform vec2 fini[1022];
                                   uniform int tabSize;
                                   varying vec2 fake_frag_coord;
                                   
                                   float makePoint(float x,float y,float sx,float sy){
                                       float xx=x+sx;
                                       float yy=y+sy;
                                       return 1.0/sqrt(xx*xx+yy*yy);
                                   }
                                   vec4 gu(vec4 a,vec4 b,float f){
                                       if(b.w==2.00)
                                           return vec4(mix(a.xyz,b.xyz,(f-a.w)*(1.0/(b.w-a.w))),0.0);
                                       else {
                                           return vec4(mix(a.xyz,b.xyz,(f-a.w)*(1.0/(b.w-a.w))),a.w);
                                       }
                                   }
                                   vec4 grad(float f){
                                       vec4 c01=vec4(1.0,1.0,1.0,0.00);
                                       vec4 c02=vec4(1.0,1.0,1.0,0.40);
                                       vec4 c03=vec4(0.75,1.0,0.0,0.70);
                                       vec4 c04=vec4(0.0,1.0,0.0,0.80);
                                       vec4 c05=vec4(0.0,0.5,0.0,0.97);
                                       vec4 c06=vec4(0.0,0.0,0.0,1.00);
                                       return (f<c02.w)?gu(c01,c02,f):
                                       (f<c03.w)?gu(c02,c03,f):
                                       (f<c04.w)?gu(c03,c04,f):
                                       (f<c05.w)?gu(c04,c05,f):
                                       gu(c05,c06,f);
                                   }
                                   void main( void ) {
                                       vec2 p=(gl_FragCoord.xy/resolution.x)*2.0-vec2(1.0,resolution.y/resolution.x);
                                       
                                       float x=p.x;
                                       float y=p.y;
                                       
                                       float a = 0.0;
                                       for (int i=0; i<tabSize; i++) {
                                           a=a+makePoint(x,y,tab[i].x,tab[i].y);
                                       }
                                       vec4 a1=grad(a/2500.0);
                                       gl_FragColor = vec4(a1.x,a1.y,a1.z,a1.w);
                                   }
                                );
        
    }
    void begin(int _texNum = 0 ) {
        if ((_texNum < nTextures) && ( _texNum >= 0)){
            ofPushStyle();
            ofPushMatrix();
            textures[_texNum].begin();
            //ofClear(0,0);
        }
    }
    void end(int _texNum = 0) {
        if ((_texNum < nTextures) && ( _texNum >= 0)){
            textures[_texNum].end();
            ofPopMatrix();
            ofPopStyle();
        }
    }
    void setTexture(ofTexture& tex, int _texNum){
        if ((_texNum < nTextures) && ( _texNum >= 0)){
            textures[_texNum].begin();
            ofClear(0,0);
            ofSetColor(255);
            tex.draw(0,0);
            textures[_texNum].end();
        }
    };
    void update(){
        ofPushStyle();
        ofSetColor(255,255);
        
        // This process is going to be repeated as many times passes variable specifies
        for(int pass = 0; pass < passes; pass++) {
            
            // All the processing is done on the pingPong ofxSwapBuffer (basicaly two ofFbo that have a swap() funtion)
            pingPong.dst->begin();
            
            ofClear(0,0);
            ofDisableAlphaBlending(); // Defer alpha blending until .draw() to keep transparencies clean.
            shader.begin();
            
            // The other ofFbo of the ofxSwapBuffer can be accessed by calling the unicode "backbuffer"
            // This is usually used to access "the previous pass", or the original frame for the first pass.
            if (pass == 0 && nTextures >= 1){
                shader.setUniformTexture("backbuffer", textures[0].getTexture(), 0);
            } else {
                shader.setUniformTexture("backbuffer", pingPong.src->getTexture(), 0);
            }
            
            // All the necessary textures are provided to the shader in this loop
            for( int i = 0; i < nTextures; i++){
                string texName = "tex" + ofToString(i);
                shader.setUniformTexture(texName.c_str(), textures[i].getTexture(), i+1 );
                string texRes = "size" + ofToString(i);
                shader.setUniform2f(texRes.c_str() , (float)textures[i].getWidth(), (float)textures[i].getHeight());
            }
            
            // There are some standard variables that are passes to the shaders. These follow the names used by
            // Ricardo Caballero's webGL Sandbox http://mrdoob.com/projects/glsl_sandbox/ and
            // ShaderToy by Inigo Quilez http://www.iquilezles.org/apps/shadertoy/ webGL interactive GLSL editors.
            shader.setUniform1f("time", ofGetElapsedTimef() );
            shader.setUniform2f("size", (float)width, (float)height);
            shader.setUniform2f("resolution", (float)width, (float)height);
            shader.setUniform2f("mouse", (float)ofGetMouseX()/width, (float)ofGetMouseY()/height);
            shader.setUniform2fv("tab[0]",&dfv[0].x,1022);
            shader.setUniform2fv("fini[0]",&dfv2[0].x,1022);
            shader.setUniform1i("tabSize", dfvSize);
            injectUniforms();
            
            // renderFrame() is a built-in funtion of ofxFXObject that only draws a white box that
            // functions as a frame where the textures can rest.
            // If you want to distort the points of a texture, you probably want to re-define the renderFrame funtion.
            renderFrame();
            
            shader.end();
            
            pingPong.dst->end();
            
            // Swap the ofFbos. Now dst is src and src is dst. Each iteration writes to
            // dst and uses src as a backbuffer, where the previous frame is kept.
            pingPong.swap();
        }
        
        pingPong.swap(); // After the loop the finished render will be at the src ofFbo of the ofxSwapBuffer
        // this extra swap() call will put it on the dst one. Which sounds more reasonable...
        
        ofPopStyle();
        
        bUpdate = false;
    };
};

class ofxGravurFX : public ofxFXObject{
public:
    ofxGravurFX(){
        passes = 1;
        internalFormat = GL_RGBA;
        fragmentShader = STRINGIFY(
                                   uniform sampler2DRect tex0;
                                   uniform sampler2DRect tex1;
                                   varying vec2 texCoordVarying;
                                   
                                   void main(){
                                       
                                       vec4 image = texture2DRect(tex0, texCoordVarying);
                                       vec4 mask = texture2DRect(tex0, texCoordVarying);
                                       if (mask.xyz == vec3(0.0,0.0,0.0)) {
                                           image.a = max(0.0, image.a - 0.5);
                                       }else image.a = min(1.0,image.a + 0.001);

                                       /*
                                       //vec4 pix = texture2D(imageMask, gl_FragCoord);
                                       if (tex1.rgb == 0.0) {
                                           tex0.a = tex0.a - 0.5;
                                       }
                                       else{
                                           tex0.a = tex0.a + 0.1;
                                       }*/
                                   }
                                   );
        vertexShader = STRINGIFY(
                                 varying vec2 texCoordVarying;
                                 void main()
                                 {
                                     texCoordVarying = gl_MultiTexCoord0.xy;
                                     gl_Position = ftransform();
                                 }
                                 );
        
        
    }
    void setTexture(ofTexture& tex, int _texNum){
        if ((_texNum < nTextures) && ( _texNum >= 0)){
            textures[_texNum].begin();
            ofClear(0,0);
            ofSetColor(255);
            tex.draw(0,0);
            textures[_texNum].end();
        }
    };
    
    
};