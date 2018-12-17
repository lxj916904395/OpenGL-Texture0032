//
//  main.m
//  OpenGL-Texture0032
//
//  Created by zhongding on 2018/12/17.
//


#include "GLHeader.h"

GLShaderManager shaderManager;
GLFrame cameraFrame;
GLFrustum viewFrustum;
GLMatrixStack modelViewMatri;
GLMatrixStack projectionMatri;

GLGeometryTransform transformLine;

#define texture_count 4
GLuint textures[texture_count];

GLTriangleBatch bigBatch;
GLTriangleBatch earthBatch;
GLTriangleBatch tuBatch;

GLBatch floorBatch;

#define star_num 50
GLFrame starts[star_num];


bool loadTgaFile(const char*name,GLenum minFilter , GLenum magFilter, GLenum wrapMode){
    
    GLbyte *pBytes;
    GLint width, heiht, componets;
    GLenum format;
    
    pBytes = gltReadTGABits(name, &width, &heiht, &componets, &format);
    
    if(pBytes == NULL){
        printf("加载tga文件出错");
        return false;
    }
    
    //设置过滤方式
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    
    //设置环绕方式
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    
    //加载纹理数据
    //参数3：可替换为GL_COMPRESSED_RGB
    glTexImage2D(GL_TEXTURE_2D, 0, componets, width, heiht, 0, format, GL_UNSIGNED_BYTE, pBytes);
    
    //精密包装,使显示的纹理更加接近纹理文件值
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    if (minFilter == GL_LINEAR_MIPMAP_LINEAR ||
        minFilter == GL_LINEAR_MIPMAP_NEAREST ||
        minFilter == GL_NEAREST_MIPMAP_LINEAR ||
        minFilter == GL_NEAREST_MIPMAP_NEAREST)
        //设置mipmap贴图
        glGenerateMipmap(GL_TEXTURE_2D);
    
    free(pBytes);
    
    return true;
}


void setup(void){
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    shaderManager.InitializeStockShaders();
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    
    //往地板floorBatch批处理中添加顶点数据
    GLfloat texSize = 1;
    floorBatch.Begin(GL_TRIANGLE_FAN, 4,1);
    floorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    floorBatch.Vertex3f(-20.f, -0.41f, 30.0f);
    
    floorBatch.MultiTexCoord2f(0, texSize, 0.0f);
    floorBatch.Vertex3f(30.0f, -0.41f, -30.f);
    
    floorBatch.MultiTexCoord2f(0, texSize, texSize);
    floorBatch.Vertex3f(-20.0f, -0.41f, -30.0f);
    
    floorBatch.MultiTexCoord2f(0, 0.0f, texSize);
    floorBatch.Vertex3f(30.0f, -0.41f, 30.0f);
    floorBatch.End();
    
    gltMakeSphere(bigBatch, 0.40f, 30.0f, 30.0f);
    gltMakeSphere(earthBatch, 0.10f, 30.0f, 30.0f);
    gltMakeSphere(tuBatch, 0.20f, 30.0f, 30.0f);

    
    glGenTextures(texture_count, textures);
    
    //地板
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    loadTgaFile("Marble.tga",GL_LINEAR_MIPMAP_LINEAR,GL_LINEAR,GL_REPEAT);
    
    //大球
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    loadTgaFile("son.tga",GL_LINEAR_MIPMAP_LINEAR,GL_LINEAR,GL_CLAMP_TO_EDGE);
    
    //小球
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    loadTgaFile("moonlike.tga",GL_LINEAR_MIPMAP_LINEAR,GL_LINEAR,GL_CLAMP_TO_EDGE);
    
    //小球
    glBindTexture(GL_TEXTURE_2D, textures[3]);
    loadTgaFile("Marslike.tga",GL_LINEAR_MIPMAP_LINEAR,GL_LINEAR,GL_CLAMP_TO_EDGE);

    
    for (int i = 0; i < star_num; i++) {
        float x = float((rand() % 300 -200) *0.1);
        float z = float((rand() % 800 -300) *0.1);
        float y = float((rand() % 300 -200) *0.1);

        starts[i].SetOrigin(x,y,z);
    }
    
    cameraFrame.MoveForward(-8);
}

void changeSize(int w, int h){
    glViewport(0, 0, w, h);
    viewFrustum.SetPerspective(35, float(w)/float(h), 1, 100);
    
    projectionMatri.LoadMatrix(viewFrustum.GetProjectionMatrix());
    transformLine.SetMatrixStacks(modelViewMatri, projectionMatri);
    
    modelViewMatri.LoadIdentity();
}

void specailKeys(int key, int x, int y){
    switch (key) {
        case GLUT_KEY_LEFT:
            cameraFrame.RotateWorld(m3dDegToRad(5), 0, 1, 0);
            break;
            
        case GLUT_KEY_RIGHT:
            cameraFrame.RotateWorld(m3dDegToRad(-5), 0, 1, 0);
            break;
            
        case GLUT_KEY_UP:
            cameraFrame.MoveForward(1);

            break;
        
        case GLUT_KEY_DOWN:
            cameraFrame.MoveForward(-1);

            break;
        default:
            break;
    }
    glutPostRedisplay();
}

static GLfloat vFloorColor[] = { 1.0f, 1.0f, 0.0f, 0.6f};
static GLfloat withte[] = { 1.0f, 1.0f, 1.0f, 1.f};
static GLfloat vLightPos[] = { 1.0f, 1.50f, 0.0f, 1.0f };

void jingxiang(GLfloat yrot){
    
    M3DVector4f lighTransform;
    
    M3DMatrix44f mcamera;
    modelViewMatri.GetMatrix(mcamera);
    m3dTransformVector4(lighTransform, vLightPos, mcamera);
    
    modelViewMatri.PushMatrix();
    
    modelViewMatri.Translatev(vLightPos);
    shaderManager.UseStockShader(GLT_SHADER_FLAT,transformLine.GetModelViewProjectionMatrix(),withte);
    earthBatch.Draw();
    modelViewMatri.PopMatrix();
    
    
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    for (int i =0 ; i < star_num ; i++) {
        modelViewMatri.PushMatrix();
        modelViewMatri.MultMatrix(starts[i]);
        shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                                     modelViewMatri.GetMatrix(),
                                     transformLine.GetProjectionMatrix(),
                                     lighTransform,
                                     withte,
                                     0);
        earthBatch.Draw();
        
        modelViewMatri.PopMatrix();
    }
    
    modelViewMatri.Translate(0, 0.05, -1);
    modelViewMatri.PushMatrix();
    modelViewMatri.Rotate(yrot, 0,1, 0);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                               modelViewMatri.GetMatrix(),
                               transformLine.GetProjectionMatrix(),
                               lighTransform,withte,0);
    bigBatch.Draw();
    modelViewMatri.PopMatrix();
    
    
    modelViewMatri.PushMatrix();
    modelViewMatri.Rotate(yrot*2.f, 0, 1, 0);
    modelViewMatri.Translate(1, 0, 0);
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                                 modelViewMatri.GetMatrix(),
                                 transformLine.GetProjectionMatrix(),
                                 lighTransform,
                                 withte,
                                 0);
    
    earthBatch.Draw();
    modelViewMatri.PopMatrix();


    modelViewMatri.PushMatrix();
    
    modelViewMatri.Rotate(yrot*1.2, 0, 1, 0);
    modelViewMatri.Translate(2.5, 0.2, 1);
    glBindTexture(GL_TEXTURE_2D, textures[3]);
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                                 modelViewMatri.GetMatrix(),
                                 transformLine.GetProjectionMatrix(),
                                 lighTransform,
                                 withte,
                                 0);
    
    tuBatch.Draw();
    
    modelViewMatri.PopMatrix();
}


void renderScene(void){
    
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    
    static CStopWatch timer;
    GLfloat yrot = timer.GetElapsedSeconds() *60;
    
    
    modelViewMatri.PushMatrix();
    
    M3DMatrix44f mcamera;
    cameraFrame.GetCameraMatrix(mcamera);
    modelViewMatri.MultMatrix(mcamera);

    
    modelViewMatri.PushMatrix();
    modelViewMatri.Scale(1, -1, 1);
    modelViewMatri.Translate(0, 0.9, 0);
    glFrontFace(GL_CW);
    jingxiang(yrot);
    glFrontFace(GL_CCW);
    modelViewMatri.PopMatrix();
    
    
    //开启混合功能
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, textures[0]);
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE,transformLine.GetModelViewProjectionMatrix(),vFloorColor,0);
    floorBatch.Draw();
    
    glDisable(GL_BLEND);
    
    jingxiang(yrot);

    modelViewMatri.PopMatrix();
    
    
    glutSwapBuffers();
    glutPostRedisplay();
}


int main(int argc, char * argv[]) {
    
    gltSetWorkingDirectory(argv[0]);
    
    glutInit(&argc, argv);
    
    glutInitWindowSize(800, 600);
    glutCreateWindow("公转自转-纹理");
    
    glutDisplayFunc(renderScene);
    glutSpecialFunc(specailKeys);
    glutReshapeFunc(changeSize);
    
    
    GLenum err = glewInit();
    if(err != GLEW_OK){
        return 1;
    }
    
    setup();
    glutMainLoop();
    
    glDeleteTextures(texture_count, textures);
    
    return 0;
}
